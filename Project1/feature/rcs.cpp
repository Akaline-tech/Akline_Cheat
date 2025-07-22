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
bool doneClientdll = false;

void NormalizePitch(float& pPitch) {
	pPitch = (pPitch < -89.0f) ? -89.0f : pPitch;

	pPitch = (pPitch > 89.f) ? 89.0f : pPitch;
}

void NormalizeYaw(float& pYaw) {
	while (pYaw > 180.f) pYaw -= 360.f ;

	while (pYaw < -180.f)  pYaw += 360.f;
}

void draw_rcs() {
    static Vector3 old_aimpunch = { 0.f, 0.f, 0.f };

    doneClientdll = true;
    auto client = reinterpret_cast<uintptr_t>(GetModuleHandle(L"client.dll"));
    if (!client)
        return;

    auto local_PlayerPawn = *reinterpret_cast<uintptr_t*>(client + cs2_dumper::offsets::client_dll::dwLocalPlayerPawn);
    if (local_PlayerPawn == 0)
        return;

    auto health = *reinterpret_cast<int*>(local_PlayerPawn + cs2_dumper::schemas::client_dll::C_BaseEntity::m_iHealth);
    if (health <= 0) {
        old_aimpunch = { 0.f, 0.f, 0.f };
        return;
    }

    auto shotfired = *reinterpret_cast<int*>(local_PlayerPawn + cs2_dumper::schemas::client_dll::C_CSPlayerPawn::m_iShotsFired);
    auto aimpunchAngles = *reinterpret_cast<Vector3*>(local_PlayerPawn + cs2_dumper::schemas::client_dll::C_CSPlayerPawn::m_aimPunchAngle);
    Vector3* local_view_angles = reinterpret_cast<Vector3*>(client + cs2_dumper::offsets::client_dll::dwViewAngles);

    if (shotfired > 1) {
        // ��ȷ��RCS��ʽ���½Ƕ� = ��ǰ�Ƕ� + 2*(��һ֡������ - ��ǰ������)
        Vector3 new_view_angles{
            local_view_angles->x + main::visuals::yA * (old_aimpunch.x - aimpunchAngles.x),
            local_view_angles->y + main::visuals::xA * (old_aimpunch.y - aimpunchAngles.y),
            0.f
        };

        // �Ƕȹ�һ��
        NormalizePitch(new_view_angles.x);
        NormalizeYaw(new_view_angles.y);

        // д�����ӽ�
        *local_view_angles = new_view_angles;

        // ���浱ǰ����������һ֡ʹ��
        old_aimpunch = aimpunchAngles;
    }
    else {
        // ���ú�����׷��
        old_aimpunch = { 0.f, 0.f, 0.f };
    }
}