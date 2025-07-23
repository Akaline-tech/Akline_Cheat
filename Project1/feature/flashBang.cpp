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



static uintptr_t GetBaseEntity(int index, uintptr_t client)//实体列表
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

static uintptr_t GetBaseEntityFromHandle(uint32_t uHandle, uintptr_t client) {

	auto entListBase = *reinterpret_cast<std::uintptr_t*>(client + cs2_dumper::offsets::client_dll::dwEntityList);
	if (entListBase == 0) {
		return 0;
	}

	const int nIndex = uHandle & 0x7FFF;

	auto entitylistbase = *reinterpret_cast<std::uintptr_t*>(entListBase + 8 * (nIndex >> 9) + 16);
	if (entitylistbase == 0) {
		return 0;
	}

	return *reinterpret_cast<std::uintptr_t*>(entitylistbase + 0x78 * (nIndex & 0x1FF));

}



void AntiFlashBang() {

	auto client = reinterpret_cast<uintptr_t>(GetModuleHandle(L"client.dll"));//模块地址获取
	if (!client)
		return;


	auto local_ctrl = *reinterpret_cast<uintptr_t*>(client + cs2_dumper::offsets::client_dll::dwLocalPlayerController);//本地玩家控制器
	if (!local_ctrl)
		return;

	auto local_hpawn = *reinterpret_cast<uint32_t*>(local_ctrl + cs2_dumper::schemas::client_dll::CBasePlayerController::m_hPawn);
	if (local_hpawn == 0xFFFFFFFF)
		return;

	auto localpawn = GetBaseEntityFromHandle(local_hpawn, client);//本地玩家
	if (!localpawn)
		return;
	/////
	auto m_flFlashBangTime = reinterpret_cast<float*>(localpawn + cs2_dumper::schemas::client_dll::C_CSPlayerPawnBase::m_flFlashBangTime);
	if (!m_flFlashBangTime)
		return;

	auto m_flFlashMaxAlpha = reinterpret_cast<float*>(localpawn + cs2_dumper::schemas::client_dll::C_CSPlayerPawnBase::m_flFlashMaxAlpha);
	if (!m_flFlashMaxAlpha)
		return;

	if (main::visuals::AntiFlash) {
		*m_flFlashBangTime = 0;
	}

	auto local_PlayerPawn = *reinterpret_cast<uintptr_t*>(client + cs2_dumper::offsets::client_dll::dwLocalPlayerPawn);
	if (local_PlayerPawn == 0)
		return;
	
	*m_flFlashMaxAlpha = main::visuals::FlashMaxAlpha * 2.5;
}