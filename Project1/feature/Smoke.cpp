#include "windows.h"
#include <cstdint>
#include <optional>
#include "../cs2dumper/offsets.hpp"
#include "../cs2dumper/client_dll.hpp"
#include "../Vec/Vector.h"
#include <cmath>
#include <imgui.h>
#include "gui.h"
#include <vector>
#include <esp.h>
#include <string>
#include <function.h>
#include <unordered_map>
#include <unordered_set>
#include <chrono>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// 全局容器存储烟雾弹的开始时间和唯一标识符
static std::unordered_map<uintptr_t, std::chrono::steady_clock::time_point> smokeStartTimes;
static std::unordered_set<uintptr_t> expiredSmokes;
static uintptr_t GetBaseEntity(int index, uintptr_t client)
{
    auto entListBase = *reinterpret_cast<std::uintptr_t*>(client + cs2_dumper::offsets::client_dll::dwEntityList);
    if (entListBase == 0) {
        return 0;
    }

    auto entitylistbase = *reinterpret_cast<std::uintptr_t*>(entListBase + 0x8 * (index >> 9) + 16);
    if (entitylistbase == 0) {
        return 0;
    }

    return *reinterpret_cast<std::uintptr_t*>(entitylistbase + (0x78 * (index & 0x1FF)));
}

void AntiSmoke() {
    const auto client = reinterpret_cast<uintptr_t>(GetModuleHandle(L"client.dll"));
    auto Matrix = reinterpret_cast<float*>(client + cs2_dumper::offsets::client_dll::dwViewMatrix);
    if (!Matrix) return;

    auto now = std::chrono::steady_clock::now(); // 获取当前时间
    std::unordered_set<uintptr_t> activeSmokes; // 存储当前有效的烟雾弹实体

    for (int i = 64; i <= 1024; i++) {
        auto C_BaseEntity = GetBaseEntity(i, client);
        if (C_BaseEntity == 0) continue;

        auto temp_1 = *reinterpret_cast<std::uintptr_t*>(C_BaseEntity + 0x10);
        if (temp_1 == 0) continue;

        auto smoke_name = *reinterpret_cast<std::uintptr_t*>(temp_1 + 0x20);
        if (smoke_name == 0) continue;

        char ch;
        std::string entity_name;
        int maxsize = 64;
        for (int i_name = 0; i_name < maxsize; i_name++) {
            ch = *reinterpret_cast<char*>(smoke_name + i_name);
            if (ch == '\0') break;
            entity_name += ch;
        }

        static const float w = ImGui::GetIO().DisplaySize.x;
        static const float h = ImGui::GetIO().DisplaySize.y;

        if (entity_name == "smokegrenade_projectile") {
            auto m_nSmokeEffectTickBegin = reinterpret_cast<int*>(C_BaseEntity + cs2_dumper::schemas::client_dll::C_SmokeGrenadeProjectile::m_nSmokeEffectTickBegin);
            if (!m_nSmokeEffectTickBegin)
                continue;
            auto m_bDidSmokeEffect = reinterpret_cast<bool*>(C_BaseEntity + cs2_dumper::schemas::client_dll::C_SmokeGrenadeProjectile::m_bDidSmokeEffect);//smokeIO
            auto m_bSmokeEffectSpawned = reinterpret_cast<bool*>(C_BaseEntity + cs2_dumper::schemas::client_dll::C_SmokeGrenadeProjectile::m_bSmokeEffectSpawned);
            auto m_vSmokeDetonationPos = *reinterpret_cast<Vector3*>(C_BaseEntity + cs2_dumper::schemas::client_dll::C_SmokeGrenadeProjectile::m_vSmokeDetonationPos);
            auto m_vSmokeColor = reinterpret_cast<Vector3*>(C_BaseEntity + cs2_dumper::schemas::client_dll::C_SmokeGrenadeProjectile::m_vSmokeColor);

            Vector3 smokeScreenPos;
            // 修复：移除错误的分号
            if (!WorldToScreen(m_vSmokeDetonationPos, smokeScreenPos, Matrix, w, h));

            // 标记当前烟雾弹为活跃状态
            activeSmokes.insert(C_BaseEntity);

            // 关键修复：检查烟雾是否已过期
            if (expiredSmokes.find(C_BaseEntity) != expiredSmokes.end()) {
                continue; // 跳过已过期的烟雾弹
            }

            // 关键修复：使用正确的标志判断烟雾是否已生成
            if (*m_bDidSmokeEffect && main::visuals::Smoketimer) {
                // 如果这是第一次检测到烟雾生成，记录开始时间
                if (smokeStartTimes.find(C_BaseEntity) == smokeStartTimes.end()) {
                    smokeStartTimes[C_BaseEntity] = now;
                }

                // 计算剩余时间
                auto startTime = smokeStartTimes[C_BaseEntity];
                auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime).count() / 1000.0f;
                float remaining = 20.9f - elapsed;

                // 烟雾已过期：清除计时器并标记为已过期
                if (remaining <= 0.0f) {
                    smokeStartTimes.erase(C_BaseEntity);
                    expiredSmokes.insert(C_BaseEntity); // 标记为已过期
                    continue; // 跳过后续绘制
                }
                // 绘制圆形计时器
                const float radius = 20.0f; // 计时器半径
                const float thickness = 4.0f; // 进度条厚度
                const ImVec2 center(smokeScreenPos.x, smokeScreenPos.y);

                // 1. 绘制背景圆环（灰色半透明）
                ImGui::GetBackgroundDrawList()->AddCircle(
                    center,
                    radius,
                    ImColor(100, 100, 100, 150), // 灰色半透明
                    thickness
                );

                // 2. 计算进度条角度（从12点位置逆时针减少）
                float progress = remaining / 20.0f; // 进度比例 (1.0 - 0.0)
                float startAngle = -static_cast<float>(M_PI) / 2.0f; // 12点位置 (-90度)
                float endAngle = startAngle - 2.0f * static_cast<float>(M_PI) * progress; // 逆时针方向

                // 3. 绘制进度条（绿色半透明）
                if (progress > 0.0f) {
                    // 根据剩余时间调整颜色（最后5秒变红）
                    ImColor progressColor;
                    if (remaining > 5.0f) {
                        progressColor = ImColor(0, 255, 0, 200); // 绿色
                    }
                    else {
                        // 剩余时间越少，红色越亮
                        float intensity = (5.0f - remaining) / 5.0f;
                        progressColor = ImColor(
                            255,
                            static_cast<int>(255 * (1.0f - intensity)),
                            0,
                            200
                        );
                    }

                    // 绘制进度弧线
                    ImGui::GetBackgroundDrawList()->PathArcTo(
                        center,
                        radius,
                        startAngle,
                        endAngle,
                        32 // 分段数
                    );
                    ImGui::GetBackgroundDrawList()->PathStroke(
                        progressColor,
                        false,
                        thickness
                    );
                }

                // 4. 在中心显示剩余秒数
                if (remaining > 0.1f) {
                    char timeText[8];
                    snprintf(timeText, sizeof(timeText), "%.0f", remaining);

                    // 计算文本位置（居中）
                    ImVec2 textSize = ImGui::CalcTextSize(timeText);
                    ImVec2 textPos(center.x - textSize.x / 2, center.y - textSize.y / 2);

                    // 绘制白色文本
                    ImGui::GetBackgroundDrawList()->AddText(
                        textPos,
                        ImColor(255, 255, 255, 220),
                        timeText
                    );
                }
            }
            else {
                // 如果烟雾未生成或已消失，从计时器中移除
                if (smokeStartTimes.find(C_BaseEntity) != smokeStartTimes.end()) {
                    smokeStartTimes.erase(C_BaseEntity);
                }
            }

            if (main::visuals::AntiSmoke) {
                *m_nSmokeEffectTickBegin = 0;
            }

            // 绘制烟雾光环
            if (*m_bDidSmokeEffect && main::visuals::SmokeHalo) {
                std::vector<ImVec2> screenPoints;
                screenPoints.reserve(main::visuals::segments);

                for (int j = 0; j < main::visuals::segments; ++j) {
                    const float angle = 2.0f * static_cast<float>(M_PI) * j / main::visuals::segments;
                    Vector3 circlePoint = m_vSmokeDetonationPos;
                    circlePoint.x += 190.f * cos(angle);
                    circlePoint.y += 190.f * sin(angle);

                    Vector3 screenPoint;
                    if (WorldToScreen(circlePoint, screenPoint, Matrix, w, h)) {
                        screenPoints.push_back(ImVec2(screenPoint.x, screenPoint.y));
                    }
                }

                if (screenPoints.size() > 2) {
                    ImGui::GetBackgroundDrawList()->AddPolyline(
                        screenPoints.data(),
                        screenPoints.size(),
                        ImColor(main::visuals::smoke_halo_color),
                        true,
                        1.5f
                    );
                }
            }

            // 更改烟雾颜色
            ImVec4 smoke_color = main::visuals::smoke_color;
            if (main::visuals::SmokeColorChanger) {
                *m_vSmokeColor = Vector3(smoke_color.x, smoke_color.y, smoke_color.z);
            }
        }
    }

    // 清理过期的计时器（防止内存泄漏）
    for (auto it = smokeStartTimes.begin(); it != smokeStartTimes.end();) {
        // 检查实体是否仍然活跃或已超时
        if (activeSmokes.find(it->first) == activeSmokes.end() ||
            std::chrono::duration_cast<std::chrono::seconds>(now - it->second).count() > 25) {
            it = smokeStartTimes.erase(it);
        }
        else {
            ++it;
        }
    }

    // 清理已过期的烟雾弹记录
    for (auto it = expiredSmokes.begin(); it != expiredSmokes.end();) {
        if (activeSmokes.find(*it) == activeSmokes.end()) {
            it = expiredSmokes.erase(it);
        }
        else {
            ++it;
        }
    }
}