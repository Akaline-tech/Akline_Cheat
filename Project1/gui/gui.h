#pragma once
namespace main {
	namespace visuals {

		inline bool box = false;
		inline ImVec4 box_color = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
		inline bool bone = false;
		inline float Box_Thickness = 1.5f;
		inline bool boxFill = false;
		inline ImVec4 Fill_color = ImVec4(1.0f, 0.0f, 0.0f, 0.2f);

		inline bool name = false;
		inline bool Reload = false;
		inline bool halo = false;
		inline ImVec4 halo_color = ImVec4(1.0f, 0.74f, 0.0f, 0.98f);
		inline ImVec4 smoke_halo_color = ImVec4(0.5f, 0.5f, 0.5f, 0.98f);
		inline ImVec4 smoke_color = ImVec4(1.0f, 0.74f, 0.0f,0.50f);
		inline float radius = 20.0f; // 圆的半径（游戏单位）
		inline int segments = 48;    // 圆的分段数（越高越平滑）
		inline bool head = false;

		inline bool Line = false;
		inline float Line_Thickness = 2.0f;
		inline bool team = true;
		inline bool health = true;
		inline float xA = 2;
		inline float yA = 2;
		static const char* LineModel[] = {"Up", "Down"};
		static int LineModel_current = 1;

		inline bool AntiSmoke = false;
		inline bool SmokeColorChanger = false;
		inline bool SmokeHalo = false;
	}
	namespace rcs {

		inline bool rcs = false;
		inline int start_ammor = 1;
	}
}

void draw_Menu();