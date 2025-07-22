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



static uintptr_t GetBaseEntity(int index, uintptr_t client)//ʵ���б�
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



Vector3 BonePos(uintptr_t addr, int32_t index) {
	int32_t d = 32 * index;
	uintptr_t address{};
	address = *reinterpret_cast<uintptr_t*>(addr + cs2_dumper::schemas::client_dll::C_BaseEntity::m_pGameSceneNode);
	if (!address)
	{
		return Vector3();
	}

	auto BoneArray = cs2_dumper::schemas::client_dll::CSkeletonInstance::m_modelState + 0x80;

	address = *reinterpret_cast<uintptr_t*>(address + BoneArray);
	if (!address)
	{
		return Vector3();
	}
	return *reinterpret_cast<Vector3*>(address + d);
}

void DrawLine(std::vector<Vector3> list, ImColor Color, float* Matrix) {

	Vector3 drawpos;
	std::vector<Vector3>Drawlist{};
	for (int i = 0; i < list.size(); ++i) {
		if (!WorldToScreen(list[i], drawpos, Matrix, ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y))
			continue;

		Drawlist.push_back(drawpos);
	}

	for (int i = 1; i < Drawlist.size(); ++i) {//fuck you cs2
		ImGui::GetBackgroundDrawList()->AddLine(ImVec2(Drawlist[i].x, Drawlist[i].y), ImVec2(Drawlist[i - 1].x, Drawlist[i - 1].y), Color);
		//ImGui::GetBackgroundDrawList()->AddCircleFilled(ImVec2(Drawlist[i].x, Drawlist[i].y), 1.2f, Color);
		//ImGui::GetBackgroundDrawList()->AddCircleFilled(ImVec2(Drawlist[0].x, Drawlist[0].y), 3.0f, Color);
	}
}

void Bone_Start(uintptr_t pawn, ImColor BoneColor, float* Matrix) {

	BoneDrawList.clear();
	BoneDrawList.push_back(BonePos(pawn, Bone_Base::BoneIndex::head_0));
	BoneDrawList.push_back(BonePos(pawn, Bone_Base::BoneIndex::neck_0));
	BoneDrawList.push_back(BonePos(pawn, Bone_Base::BoneIndex::spine_2));
	BoneDrawList.push_back(BonePos(pawn, Bone_Base::BoneIndex::pelvis));
	DrawLine(BoneDrawList, BoneColor, Matrix);

	BoneDrawList.clear();
	BoneDrawList.push_back(BonePos(pawn, Bone_Base::BoneIndex::neck_0));
	BoneDrawList.push_back(BonePos(pawn, Bone_Base::BoneIndex::arm_upper_l));
	BoneDrawList.push_back(BonePos(pawn, Bone_Base::BoneIndex::arm_lower_l));
	BoneDrawList.push_back(BonePos(pawn, Bone_Base::BoneIndex::hand_l));
	DrawLine(BoneDrawList, BoneColor, Matrix);

	BoneDrawList.clear();
	BoneDrawList.push_back(BonePos(pawn, Bone_Base::BoneIndex::neck_0));
	BoneDrawList.push_back(BonePos(pawn, Bone_Base::BoneIndex::arm_upper_r));
	BoneDrawList.push_back(BonePos(pawn, Bone_Base::BoneIndex::arm_lower_r));
	BoneDrawList.push_back(BonePos(pawn, Bone_Base::BoneIndex::hand_r));
	DrawLine(BoneDrawList, BoneColor, Matrix);

	BoneDrawList.clear();
	BoneDrawList.push_back(BonePos(pawn, Bone_Base::BoneIndex::pelvis));
	BoneDrawList.push_back(BonePos(pawn, Bone_Base::BoneIndex::leg_upper_l));
	BoneDrawList.push_back(BonePos(pawn, Bone_Base::BoneIndex::leg_lower_l));
	BoneDrawList.push_back(BonePos(pawn, Bone_Base::BoneIndex::ankle_l));
	DrawLine(BoneDrawList, BoneColor, Matrix);

	BoneDrawList.clear();
	BoneDrawList.push_back(BonePos(pawn, Bone_Base::BoneIndex::pelvis));
	BoneDrawList.push_back(BonePos(pawn, Bone_Base::BoneIndex::leg_upper_r));
	BoneDrawList.push_back(BonePos(pawn, Bone_Base::BoneIndex::leg_lower_r));
	BoneDrawList.push_back(BonePos(pawn, Bone_Base::BoneIndex::ankle_r));
	DrawLine(BoneDrawList, BoneColor, Matrix);

}

std::optional<Vector3> GetEyePos(uintptr_t addr) noexcept {//�����������λ
	auto* Origin = reinterpret_cast<Vector3*>(addr + cs2_dumper::schemas::client_dll::C_BasePlayerPawn::m_vOldOrigin);
	auto* ViewOffset = reinterpret_cast<Vector3*>(addr + cs2_dumper::schemas::client_dll::C_BaseModelEntity::m_vecViewOffset);

	Vector3 LocalEye = *Origin + *ViewOffset;
	if (!std::isfinite(LocalEye.x) || !std::isfinite(LocalEye.y) || !std::isfinite(LocalEye.z))
		return std::nullopt;

	if (LocalEye.Length() < 0.1f)
		return std::nullopt;

	return LocalEye;

}


void draw_esp() {

	auto client = reinterpret_cast<uintptr_t>(GetModuleHandle(L"client.dll"));//ģ���ַ��ȡ
	if (!client)
		return;

	//
	auto dwGameEntitySystem = *reinterpret_cast<std::uintptr_t*>(client + cs2_dumper::offsets::client_dll::dwEntityList);

	auto dwGameEntitySystem_highestyIndex = *reinterpret_cast<int*>(client + cs2_dumper::offsets::client_dll::dwGameEntitySystem_highestEntityIndex);
	//

	auto local_ctrl = *reinterpret_cast<uintptr_t*>(client + cs2_dumper::offsets::client_dll::dwLocalPlayerController);//������ҿ�����
	if (!local_ctrl)
		return;

	auto local_hpawn = *reinterpret_cast<uint32_t*>(local_ctrl + cs2_dumper::schemas::client_dll::CBasePlayerController::m_hPawn);
	if (local_hpawn == 0xFFFFFFFF)
		return;

	auto localpawn = GetBaseEntityFromHandle(local_hpawn, client);//�������
	if (!localpawn)
		return;

	auto local_team = *reinterpret_cast<int*>(localpawn + cs2_dumper::schemas::client_dll::C_BaseEntity::m_iTeamNum);//��ȡ����
	if (!local_team)
		return;

	auto Matrix = reinterpret_cast<float*>(client + cs2_dumper::offsets::client_dll::dwViewMatrix);
	if (!Matrix)
		return;

	for (int i = 0; i < 64; i++) {//����ʵ��
		auto player_co = GetBaseEntity(i, client);

		if (!player_co)
			continue;

		auto player_hpawn = *reinterpret_cast<uint32_t*>(player_co + cs2_dumper::schemas::client_dll::CBasePlayerController::m_hPawn);
		if (player_hpawn == 0xFFFFFFFF)
			continue;

		auto player_pawn = GetBaseEntityFromHandle(player_hpawn, client);
		if (!player_pawn)
			continue;



		if (main::visuals::team)
		{
			auto player_team = *reinterpret_cast<int*>(player_pawn + cs2_dumper::schemas::client_dll::C_BaseEntity::m_iTeamNum);//�������
			if (local_team == player_team)
				continue;
		}

		auto player_health = *reinterpret_cast<int*>(player_pawn + cs2_dumper::schemas::client_dll::C_BaseEntity::m_iHealth);//Ѫ������ ������������һ�ump45
		if (player_health <= 0)
			continue;

		const char* player_Name = reinterpret_cast<const char*>(player_co + cs2_dumper::schemas::client_dll::CBasePlayerController::m_iszPlayerName);//�������
		if (!player_Name)
			continue;

		const char* local_player_Name = reinterpret_cast<const char*>(local_ctrl + cs2_dumper::schemas::client_dll::CBasePlayerController::m_iszPlayerName);//����������
		if (local_player_Name == player_Name)
			continue;


		auto player_Origin = *reinterpret_cast<Vector3*>(player_pawn + cs2_dumper::schemas::client_dll::C_BasePlayerPawn::m_vOldOrigin);//���ԭ��

		auto localplayer_Origin = *reinterpret_cast<Vector3*>(localpawn + cs2_dumper::schemas::client_dll::C_BasePlayerPawn::m_vOldOrigin);//���ԭ��

		auto player_eyepos_op_vec = GetEyePos(player_pawn);

		if (!player_eyepos_op_vec.has_value())
			continue;

		auto player_eyepos = player_eyepos_op_vec.value();

		static const float w = ImGui::GetIO().DisplaySize.x;
		static const float h = ImGui::GetIO().DisplaySize.y;

		Vector3 head_pos_2d{};
		Vector3 abs_pos_2d{};

		uintptr_t local_pWeapon = *reinterpret_cast<uintptr_t*>(localpawn + cs2_dumper::schemas::client_dll::C_CSPlayerPawnBase::m_pClippingWeapon);

		auto WeaponName = *reinterpret_cast<char*>(local_pWeapon + cs2_dumper::schemas::client_dll::C_CSPlayerPawnBase::m_pClippingWeapon);

		uintptr_t pWeapon = *reinterpret_cast<uintptr_t*>(player_pawn + cs2_dumper::schemas::client_dll::C_CSPlayerPawnBase::m_pClippingWeapon);
		if (!pWeapon)
			continue;
		auto WeaponInReload = *reinterpret_cast<bool*>(pWeapon + cs2_dumper::schemas::client_dll::C_CSWeaponBase::m_bInReload);

		if (main::visuals::halo) {


			// �洢ת�������Ļ��
			std::vector<ImVec2> screenPoints;
			screenPoints.reserve(main::visuals::segments);

			// ����Բ�ε㼯����ˮƽ���ϣ�
			for (int i = 0; i < main::visuals::segments; ++i) {
				const float angle = 2.0f * 3.1415926f * i / main::visuals::segments;
				Vector3 circlePoint = player_Origin;
				circlePoint.x += main::visuals::radius * cos(angle);
				circlePoint.y += main::visuals::radius * sin(angle);

				Vector3 screenPoint;
				if (WorldToScreen(circlePoint, screenPoint, Matrix, w, h)) {
					screenPoints.push_back(ImVec2(screenPoint.x, screenPoint.y));
				}
			}

			// ����͸��Ч����Բ�Σ���Բ��
			if (screenPoints.size() > 2) {
				ImGui::GetBackgroundDrawList()->AddPolyline(
					screenPoints.data(),
					screenPoints.size(),
					ImColor(main::visuals::halo_color), // ��ɫԲ��
					true,  // �պ���״
					1.5f   // �߿�
				);
			}
		}
		char testpos[128];

		snprintf(testpos, sizeof(testpos), u8"test %llX ", (unsigned long long)dwGameEntitySystem_highestyIndex);

		ImGui::GetBackgroundDrawList()->AddText(ImVec2(1000, 70), ImColor(255, 240, 255, 255), testpos);

		if (!WorldToScreen(player_Origin, abs_pos_2d, Matrix, w, h)) { continue; }//������Ļ�о�����
		if (!WorldToScreen(player_eyepos, head_pos_2d, Matrix, w, h)) { continue; }//������Ļ�о�����

		//�����Сλ�ü���
		const float height{ ::abs(head_pos_2d.y - abs_pos_2d.y) * 1.25f };
		const float width{ height / 2.f };
		const float x = head_pos_2d.x - (width / 2);
		const float y = head_pos_2d.y - (width / 2.5f) + width / 7 - 10 / 9;

		//Ѫ����Сλ�ü���
		const float healthBarWidth = 4.0f; const float healthBarOffset = 3.0f; const float healthBarX = x - healthBarWidth - healthBarOffset; const float healthBarHeight = height;
		const float healthRatio = player_health / 100.0f; const float filledHeight = healthBarHeight * healthRatio;
		ImU32 healthColor;
		if (player_health > 70) { healthColor = ImColor(0, 255, 0, 255); }
		else if (player_health > 30) { healthColor = ImColor(255, 255, 0, 255); }
		else { healthColor = ImColor(255, 0, 0, 255); }

		//����λ�ü���
		float nameX = x + width / 2.0f;
		float nameY = y - 15.0f; // �ڷ����Ϸ�20���ش�
		ImVec2 textSize = ImGui::CalcTextSize(player_Name);

		if (main::visuals::box && width > 1.1)
		{
			ImGui::GetBackgroundDrawList()->AddRect(ImVec2(x, y), ImVec2(x + width, y + height), ImColor(main::visuals::box_color), 0.0f, 0, main::visuals::Box_Thickness);
		}

		Vector3 headScreenPos;
		if (main::visuals::bone)
		{
			Bone_Start(player_pawn, ImColor(155, 155, 155, 240), Matrix);
		}

		if (main::visuals::name)
		{
			ImGui::GetBackgroundDrawList()->AddText(ImVec2(nameX - textSize.x / 2, nameY), ImColor(255, 255, 255, 255), player_Name);
		}

		if (main::visuals::Reload && WeaponInReload)
		{
			ImGui::GetBackgroundDrawList()->AddText(ImVec2(x + width + 7, y + height - 10), ImColor(255, 255, 255, 255), u8"װ����");
		}

		if (main::visuals::boxFill && width > 1.1)
		{
			ImGui::GetBackgroundDrawList()->AddRectFilled(ImVec2(x + 1.5f, y + 1.5f), ImVec2(x + width - 3.0f, y + height - 3.0f), ImColor(main::visuals::Fill_color), 0.0f, 0);
		}

		if (main::visuals::head)//ͷ��λ��
		{
			ImGui::GetBackgroundDrawList()->AddCircle(ImVec2(head_pos_2d.x, head_pos_2d.y), width / 8, ImColor(255, 0, 0, 255), 0.0f, 0);
		}

		if (main::visuals::Line) {
			ImGui::GetBackgroundDrawList()->AddLine(ImVec2(x + width / 2, y + height), ImVec2(1920 / 2, 1080), IM_COL32(0, 255, 0, 255), main::visuals::Line_Thickness);
		}

		if (main::visuals::health)//Ѫ��
		{
			//����Ѫ��������
			ImGui::GetBackgroundDrawList()->AddRectFilled(ImVec2(healthBarX, y), ImVec2(healthBarX + healthBarWidth, y + healthBarHeight), ImColor(50, 50, 50, 255));
			//���Ƶ�ǰѪ��
			ImGui::GetBackgroundDrawList()->AddRectFilled(ImVec2(healthBarX, y + (healthBarHeight - filledHeight)), ImVec2(healthBarX + healthBarWidth, y + healthBarHeight), healthColor);
			//����Ѫ�����߿�
			ImGui::GetBackgroundDrawList()->AddRect(ImVec2(healthBarX, y), ImVec2(healthBarX + healthBarWidth, y + healthBarHeight), ImColor(0, 0, 0, 255), 0.0f, 0, 1.0f);
		}
	}
}