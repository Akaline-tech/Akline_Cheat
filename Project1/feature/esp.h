#pragma once
#include <Vector.h>
#include <vector>
#include "../imgui_d11/imgui.h"

uintptr_t GetBaseEntity(int index, uintptr_t client);
uintptr_t GetBaseEntityFromHandle(uint32_t uHandle, uintptr_t client);
void draw_esp();

namespace Bone_Base {

	enum BoneIndex {
        pelvis = 0,//
        spine_1 = 4,//
        spine_2 = 2,//
        neck_0 = 5,//
        head_0 = 6,//
        arm_upper_l = 8,//
        arm_lower_l = 9,//
        hand_l = 10,//
        arm_upper_r = 13,//
        arm_lower_r = 14,//
        hand_r = 15,//
        leg_upper_l = 22,//
        leg_lower_l = 23,//
        ankle_l = 24,//
        leg_upper_r = 25,//
        leg_lower_r = 26,//
        ankle_r = 27,//
	};
}
Vector3 BonePos(uintptr_t addr, int32_t index);

void Bone_Start(uintptr_t pawn, ImColor BoneColor, float* Matrix);

void DrawLine(std::vector<Vector3> list, ImColor Color, float* Matrix);

inline std::vector<Vector3>BoneDrawList{};