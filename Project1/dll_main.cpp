// dll_main.cpp
#include <Windows.h>
#include <d3d11.h>
#include <MinHook.h>
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "../../Project1/imgui_d11/imgui_impl_dx11.h"
#include <thread>
#include <gui.h>
#include "feature/esp.h"
#include <rcs.h>
#include <Smoke.h>
#include <flashBang.h>
#include <bhop.h>
#include <offsets.hpp>

static ID3D11Device* g_pd3dDevice = nullptr;
static IDXGISwapChain* g_pSwapChain = nullptr;
static ID3D11DeviceContext* g_pd3dContext = nullptr;
static ID3D11RenderTargetView* view = nullptr;
static HWND g_hwnd = nullptr;
void* origin_present = nullptr;

// 添加菜单状态变量
static bool g_menuOpen = false;  // 控制菜单开关状态

using Present = HRESULT(__stdcall*)(IDXGISwapChain*, UINT, UINT);

WNDPROC origin_wndProc;

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND, UINT, WPARAM, LPARAM);

LRESULT __stdcall WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    // 处理Insert键按下事件
    if (uMsg == WM_KEYDOWN && wParam == VK_HOME) {//VK_INSERT
        g_menuOpen = !g_menuOpen;  // 切换菜单状态
    }

    // 当菜单打开时，让ImGui处理所有鼠标事件
    if (g_menuOpen && ImGui_ImplWin32_WndProcHandler(hwnd, uMsg, wParam, lParam)) {
        return true;
    }

    return CallWindowProc(origin_wndProc, hwnd, uMsg, wParam, lParam);
}

bool inited = false;
ImFont* msyhFont = nullptr;  // 全局字体指针

long __stdcall my_present(IDXGISwapChain* _this, UINT a, UINT b) {

    if (!inited) {
        _this->GetDevice(__uuidof(ID3D11Device), (void**)&g_pd3dDevice);
        g_pd3dDevice->GetImmediateContext(&g_pd3dContext);

        DXGI_SWAP_CHAIN_DESC sd;
        _this->GetDesc(&sd);
        g_hwnd = sd.OutputWindow;

        ID3D11Texture2D* buf{};
        _this->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&buf);
        g_pd3dDevice->CreateRenderTargetView(buf, nullptr, &view);
        buf->Release();

        origin_wndProc = (WNDPROC)SetWindowLongPtr(g_hwnd, GWLP_WNDPROC, (LONG_PTR)WndProc);

        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();

        // 只加载一次字体
        msyhFont = io.Fonts->AddFontFromFileTTF(
            "C:\\Windows\\Fonts\\msyh.ttc",
            18.0f,
            nullptr,
            io.Fonts->GetGlyphRangesChineseFull()
        );

        ImGui_ImplWin32_Init(g_hwnd);
        ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dContext);
        inited = true;
    }

    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();

    ImGui::NewFrame();

    // 设置光标绘制状态（菜单打开时显示光标）
    ImGui::GetIO().MouseDrawCursor = g_menuOpen;

    // 根据菜单状态决定是否绘制
    if (g_menuOpen) {
        // 使用微软雅黑字体
        ImGui::PushFont(msyhFont);
        draw_Menu();
        ImGui::PopFont();
    }
    draw_esp();
    if (main::rcs::rcs)
    {
        draw_rcs();
    }

    AntiSmoke();
    AntiFlashBang();

    ImGui::EndFrame();

    ImGui::Render();
    g_pd3dContext->OMSetRenderTargets(1, &view, nullptr);
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    return ((Present)origin_present)(_this, a, b);
}

static bool(__fastcall* fnOriginalCreateMove)(void*, int, CUserCMD*) = nullptr;
static bool __fastcall hkCreateMove(void* pCSGOInput, int nSlot, CUserCMD* pcmd) {
    bool bResult = fnOriginalCreateMove(pCSGOInput, nSlot, pcmd);
    
    bhop(pcmd);

    return bResult;
}

DWORD create(void*) {
    const unsigned level_count = 2;
    D3D_FEATURE_LEVEL levels[level_count] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0 };
    DXGI_SWAP_CHAIN_DESC sd{};
    sd.BufferCount = 1;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = GetForegroundWindow();
    sd.SampleDesc.Count = 1;
    sd.Windowed = true;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    auto hr = D3D11CreateDeviceAndSwapChain(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        0,
        levels,
        level_count,
        D3D11_SDK_VERSION,
        &sd,
        &g_pSwapChain,
        &g_pd3dDevice,
        nullptr,
        nullptr);

    if (g_pSwapChain) {
        auto vtable_ptr = (void***)(g_pSwapChain);
        auto vtable = *vtable_ptr;
        auto present = vtable[8];
        MH_Initialize();
        MH_CreateHook(present, my_present, &origin_present);
        MH_EnableHook(present);
        g_pd3dDevice->Release();
        g_pSwapChain->Release();
    }

    const auto client = reinterpret_cast<uintptr_t>(GetModuleHandle(L"client.dll"));

    void* pCCSGOInput = reinterpret_cast<void*>(client + cs2_dumper::offsets::client_dll::dwCSGOInput);

    void* pfnCreateMove = (*reinterpret_cast<void***>(pCCSGOInput))[21];

    MH_CreateHook(pfnCreateMove, hkCreateMove, reinterpret_cast<void**>(&fnOriginalCreateMove));
    MH_EnableHook(pfnCreateMove);

    return 0;
}

BOOL WINAPI DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved) {
    if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
        CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)create, NULL, 0, NULL);
        CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)draw_rcs, NULL, 0, NULL);
    }
    return TRUE;
}