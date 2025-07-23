#include "windows.h"
#include <cstdint>
#include <optional>
#include "../cs2dumper/offsets.hpp"
#include "../cs2dumper/client_dll.hpp"
#include "../Vec/Vector.h"
#include <imgui.h>
#include "gui.h"
#include <vector>
#include <esp.h>
#include <string>
#include <function.h>
#include <buttons.hpp>
#include "bhop.h"

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

void bhop(CUserCMD* pcmd)
{

	if(!main::bhop::bhop)
		return;

	auto client = reinterpret_cast<uintptr_t>(GetModuleHandle(L"client.dll"));

	auto local_ctrl = *reinterpret_cast<uintptr_t*>(client + cs2_dumper::offsets::client_dll::dwLocalPlayerController);
	if (!local_ctrl)
		return;

	auto local_hpawn = *reinterpret_cast<uint32_t*>(local_ctrl + cs2_dumper::schemas::client_dll::CBasePlayerController::m_hPawn);
	if (local_hpawn == 0xFFFFFFFF)
		return;

	auto localpawn = GetBaseEntityFromHandle(local_hpawn, client);//±¾µØÍæ¼Ò
	if (!localpawn)
		return;

	short m_fflags = *reinterpret_cast<short*>(localpawn + cs2_dumper::schemas::client_dll::C_BaseEntity::m_fFlags);

	if (m_fflags & (1 << 0)) {
		pcmd->nButtons.nValue &= ~IN_JUMP;
	}



}