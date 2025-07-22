#include <imgui.h>
#include <gui.h>
#include <offsets.hpp>
#include "windows.h"
#include <cstdint>
#include <optional>
#include "../cs2dumper/offsets.hpp"
#include "../cs2dumper/client_dll.hpp"
#include "../Vec/Vector.h"
#include <cmath>
#include <imgui.h>
#include "gui.h"

void draw_Menu() {
    // Set window rounding and fixed size
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 8.0f); // Slightly reduce corner radius
    ImGui::SetNextWindowSize(ImVec2(1000, 700), ImGuiCond_Always);
    ImGui::Begin("Akline.Tech", nullptr,
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoTitleBar);
    {
        // Set inner padding
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10, 10));
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8, 4));
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10, 10));
        ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 6.0f); // Child window rounding

        // Two-column layout: left button bar, right content area
        ImGui::Columns(2, "tabs", false);
        ImGui::SetColumnWidth(0, 150.0f); // Fixed left column width

        // Left button area
        static int selected_tab = 0; // 0=Visual, 1=Recoil
        ImGui::BeginChild("##Buttons", ImVec2(0, 0), true, ImGuiWindowFlags_AlwaysUseWindowPadding);
        {
            ImGui::Text("Akline.Tech");
            ImGui::Separator();
            // Button style: highlight selected
            ImVec2 buttonSize(-FLT_MIN, 50);

            ImGui::PushStyleColor(ImGuiCol_Button, selected_tab == 0 ?
                ImGui::GetStyle().Colors[ImGuiCol_ButtonActive] :
                ImGui::GetStyle().Colors[ImGuiCol_Button]);
            if (ImGui::Button("Visual", buttonSize)) selected_tab = 0;
            ImGui::PopStyleColor();

            ImGui::PushStyleColor(ImGuiCol_Button, selected_tab == 1 ?
                ImGui::GetStyle().Colors[ImGuiCol_ButtonActive] :
                ImGui::GetStyle().Colors[ImGuiCol_Button]);
            if (ImGui::Button("RCS", buttonSize)) selected_tab = 1;
            ImGui::PopStyleColor();
        }
        ImGui::EndChild();

        ImGui::NextColumn(); // Switch to right content column

        // Right content area - use fixed height to avoid squeezing
        ImGui::BeginChild("##Content", ImVec2(0, ImGui::GetContentRegionAvail().y), true, ImGuiWindowFlags_AlwaysUseWindowPadding);
        {
            if (selected_tab == 0) { // Visual page
                // Calculate available height minus spacing
                float content_height = ImGui::GetContentRegionAvail().y - ImGui::GetStyle().ItemSpacing.y;

                // Left: ESP settings group box (60% width)
                ImGui::BeginChild("##ESPSettings", ImVec2(ImGui::GetContentRegionAvail().x * 0.6f, content_height), true);
                {
                    // Group box title
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.2f, 0.6f, 1.0f, 1.0f));
                    ImGui::SetCursorPos(ImVec2(15, 8));
                    ImGui::Text("ESP Settings");
                    ImGui::PopStyleColor();
                    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 15);

                    // ESP settings controls
                    ImGui::Checkbox("Box ESP", &main::visuals::box);
                    ImGui::SameLine();
                    ImGui::SetNextItemWidth(150);
                    ImGui::SliderFloat("Box Thickness##1", &main::visuals::Box_Thickness, 1.0f, 5.0f);

                    ImGui::Checkbox("Fill Box", &main::visuals::boxFill);
                    ImGui::Checkbox("Skeleton", &main::visuals::bone);
                    ImGui::Checkbox("Show Name", &main::visuals::name);
                    ImGui::Checkbox("Reload Status", &main::visuals::Reload);

                    ImGui::Checkbox("Direction Line", &main::visuals::Line);
                    ImGui::SameLine();
                    ImGui::SetNextItemWidth(150);
                    ImGui::SliderFloat("Line Thickness##2", &main::visuals::Line_Thickness, 1.0f, 5.0f);

                    ImGui::Checkbox("Player Halo", &main::visuals::halo);
                    ImGui::SameLine();
                    ImGui::SetNextItemWidth(150);
                    ImGui::SliderFloat("Halo Radius", &main::visuals::radius, 5.0f, 250.0f);
                    ImGui::SameLine();
                    ImGui::SetNextItemWidth(150);
                    ImGui::SliderInt("Halo Segments", &main::visuals::segments, 12, 64);

                    ImGui::Checkbox("Head Marker", &main::visuals::head);
                    ImGui::Checkbox("Show Health", &main::visuals::health);
                    ImGui::Checkbox("Team Filter", &main::visuals::team);
                    ImGui::Checkbox("Anti-Smoke", &main::visuals::AntiSmoke);
                    ImGui::Checkbox("Smoke Halo", &main::visuals::SmokeHalo);
                    ImGui::Checkbox("Smoke Color Changer", &main::visuals::SmokeColorChanger);
                }
                ImGui::EndChild();

                ImGui::SameLine(); // Ensure right box is on same line

                // Right: Color adjustment group box (40% width)
                ImGui::BeginChild("##ColorSettings", ImVec2(0, content_height), true);
                {
                    // Group box title
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.2f, 0.6f, 1.0f, 1.0f));
                    ImGui::SetCursorPos(ImVec2(15, 8));
                    ImGui::Text("Color Settings");
                    ImGui::PopStyleColor();
                    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 15);

                    ImGui::ColorEdit4("Box Color", (float*)&main::visuals::box_color, ImGuiColorEditFlags_NoInputs);
                    ImGui::ColorEdit4("Fill Color", (float*)&main::visuals::Fill_color, ImGuiColorEditFlags_NoInputs);
                    ImGui::ColorEdit4("Halo Color", (float*)&main::visuals::halo_color, ImGuiColorEditFlags_NoInputs);
                    ImGui::ColorEdit3("Smoke Color", (float*)&main::visuals::smoke_color, ImGuiColorEditFlags_NoInputs);
                    ImGui::ColorEdit4("Smoke Halo Color", (float*)&main::visuals::smoke_halo_color, ImGuiColorEditFlags_NoInputs);
                    ImGui::Dummy(ImVec2(0, 20));
                }
                ImGui::EndChild();
            }
            else if (selected_tab == 1) { // Recoil page
                // Recoil settings group box
                ImGui::BeginChild("##AimbotSettings", ImVec2(0, 0), true);
                {
                    // Group box title
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.2f, 0.6f, 1.0f, 1.0f));
                    ImGui::SetCursorPos(ImVec2(15, 8));
                    ImGui::Text("Recoil Settings");
                    ImGui::PopStyleColor();
                    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 15);

        
                    ImGui::Separator();
                    ImGui::Checkbox("RCS", &main::rcs::rcs);
                    ImGui::SameLine();
                    ImGui::SetNextItemWidth(200);
                    ImGui::SliderInt("After Bullet Count", &main::rcs::start_ammor, 0, 30);
                    ImGui::Text("Other settings are under development.");
                    //ImGui::SliderFloat("Recoil X", &main::visuals::xA, 0.f, 5.f);Other settings are under development.
                    //ImGui::SliderFloat("Recoil Y", &main::visuals::yA, 0.f, 5.f);
                }
                ImGui::EndChild();
            }
        }
        ImGui::EndChild();

        ImGui::Columns(1); // Restore single column layout
        ImGui::PopStyleVar(5); // Restore all styles
    }
    ImGui::End();
    ImGui::PopStyleVar(); // Window rounding
}
/*
#include <imgui.h>
#include <gui.h>
#include <offsets.hpp>
#include "windows.h"
#include <cstdint>
#include <optional>
#include "../cs2dumper/offsets.hpp"
#include "../cs2dumper/client_dll.hpp"
#include "../Vec/Vector.h"
#include <cmath>
#include <imgui.h>
#include "gui.h"

void draw_Menu() {
    // 设置窗口圆角和固定大小
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 8.0f); // 稍微减少圆角半径
    ImGui::SetNextWindowSize(ImVec2(1000, 700), ImGuiCond_Always);
    ImGui::Begin(u8"Akline.Tech", nullptr,
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoTitleBar);
    {
        // 设置窗口内边距
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10, 10));
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8, 4));
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10, 10));
        ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 6.0f); // 子窗口圆角

        // 使用两列布局：左侧按钮栏，右侧内容区
        ImGui::Columns(2, "tabs", false);
        ImGui::SetColumnWidth(0, 150.0f); // 固定左侧列宽

        // 左侧按钮区域
        static int selected_tab = 0; // 0=视觉, 1=压枪
        ImGui::BeginChild("##Buttons", ImVec2(0, 0), true, ImGuiWindowFlags_AlwaysUseWindowPadding);
        {
            // 按钮样式：选中状态高亮
            ImVec2 buttonSize(-FLT_MIN, 50);

            ImGui::PushStyleColor(ImGuiCol_Button, selected_tab == 0 ?
                ImGui::GetStyle().Colors[ImGuiCol_ButtonActive] :
                ImGui::GetStyle().Colors[ImGuiCol_Button]);
            if (ImGui::Button(u8"视觉设置", buttonSize)) selected_tab = 0;
            ImGui::PopStyleColor();

            ImGui::PushStyleColor(ImGuiCol_Button, selected_tab == 1 ?
                ImGui::GetStyle().Colors[ImGuiCol_ButtonActive] :
                ImGui::GetStyle().Colors[ImGuiCol_Button]);
            if (ImGui::Button(u8"压枪设置", buttonSize)) selected_tab = 1;
            ImGui::PopStyleColor();
        }
        ImGui::EndChild();

        ImGui::NextColumn(); // 切换到右侧内容列

        // 右侧内容区域 - 使用固定高度避免挤压
        ImGui::BeginChild("##Content", ImVec2(0, ImGui::GetContentRegionAvail().y), true, ImGuiWindowFlags_AlwaysUseWindowPadding);
        {
            if (selected_tab == 0) { // 视觉页面
                // 计算可用高度并减去间距
                float content_height = ImGui::GetContentRegionAvail().y - ImGui::GetStyle().ItemSpacing.y;

                // 左侧：ESP设置分组框 (60%宽度)
                ImGui::BeginChild("##ESPSettings", ImVec2(ImGui::GetContentRegionAvail().x * 0.6f, content_height), true);
                {
                    // 分组框标题
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.2f, 0.6f, 1.0f, 1.0f));
                    ImGui::SetCursorPos(ImVec2(15, 8));
                    ImGui::Text(u8"ESP设置");
                    ImGui::PopStyleColor();
                    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 15);

                    // ESP设置控件
                    ImGui::Checkbox(u8"方框透视", &main::visuals::box);
                    ImGui::SameLine();
                    ImGui::SetNextItemWidth(150);
                    ImGui::SliderFloat(u8"方框粗细##1", &main::visuals::Box_Thickness, 1.0f, 5.0f);

                    ImGui::Checkbox(u8"填充方框", &main::visuals::boxFill);
                    ImGui::Checkbox(u8"骨骼绘制", &main::visuals::bone);
                    ImGui::Checkbox(u8"显示名称", &main::visuals::name);
                    ImGui::Checkbox(u8"装弹状态", &main::visuals::Reload);

                    ImGui::Checkbox(u8"直线指示", &main::visuals::Line);
                    ImGui::SameLine();
                    ImGui::SetNextItemWidth(150);
                    ImGui::SliderFloat(u8"直线粗细##2", &main::visuals::Line_Thickness, 1.0f, 5.0f);

                    ImGui::Checkbox(u8"角色光环", &main::visuals::halo);
                    ImGui::SameLine();
                    ImGui::SetNextItemWidth(150);
                    ImGui::SliderFloat(u8"光环半径", &main::visuals::radius, 5.0f, 250.0f);
                    ImGui::SameLine();
                    ImGui::SetNextItemWidth(150);
                    ImGui::SliderInt(u8"光环精度", &main::visuals::segments, 12, 64);

                    ImGui::Checkbox(u8"头部标记", &main::visuals::head);
                    ImGui::Checkbox(u8"显示血量", &main::visuals::health);
                    ImGui::Checkbox(u8"队伍过滤", &main::visuals::team);
                    ImGui::Checkbox(u8"屏蔽烟雾", &main::visuals::AntiSmoke);
                    ImGui::Checkbox(u8"烟雾光环", &main::visuals::SmokeHalo);
                    ImGui::Checkbox(u8"烟雾颜色变更器", &main::visuals::SmokeColorChanger);
                }
                ImGui::EndChild();

                ImGui::SameLine(); // 确保右侧框在同一行

                // 右侧：颜色调整分组框 (40%宽度)
                ImGui::BeginChild("##ColorSettings", ImVec2(0, content_height), true);
                {
                    // 分组框标题
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.2f, 0.6f, 1.0f, 1.0f));
                    ImGui::SetCursorPos(ImVec2(15, 8));
                    ImGui::Text(u8"颜色设置");
                    ImGui::PopStyleColor();
                    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 15);


                    ImGui::ColorEdit4(u8"边框颜色", (float*)&main::visuals::box_color, ImGuiColorEditFlags_NoInputs);
                    ImGui::ColorEdit4(u8"填充颜色", (float*)&main::visuals::Fill_color, ImGuiColorEditFlags_NoInputs);
                    ImGui::ColorEdit4(u8"光环颜色", (float*)&main::visuals::halo_color, ImGuiColorEditFlags_NoInputs);
                    ImGui::ColorEdit3(u8"烟雾颜色", (float*)&main::visuals::smoke_color, ImGuiColorEditFlags_NoInputs);
                    ImGui::ColorEdit4(u8"烟雾光环颜色", (float*)&main::visuals::smoke_halo_color, ImGuiColorEditFlags_NoInputs);
                    ImGui::Dummy(ImVec2(0, 20));
                }
                ImGui::EndChild();
            }
            else if (selected_tab == 1) { // 压枪页面
                // 压枪设置分组框
                ImGui::BeginChild("##AimbotSettings", ImVec2(0, 0), true);
                {
                    // 分组框标题
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.2f, 0.6f, 1.0f, 1.0f));
                    ImGui::SetCursorPos(ImVec2(15, 8));
                    ImGui::Text(u8"压枪设置");
                    ImGui::PopStyleColor();
                    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 15);

                    ImGui::Text(u8"压枪功能设置");
                    ImGui::Separator();
                    ImGui::Checkbox(u8"自动压枪", &main::rcs::rcs);
                    ImGui::SameLine();
                    ImGui::SetNextItemWidth(200);
                    ImGui::SliderInt(u8"第几发后压枪", &main::rcs::start_ammor, 0, 30);
                    ImGui::SliderFloat(u8"压枪x", &main::visuals::xA, 0.f, 5.f);
                    ImGui::SliderFloat(u8"压枪y", &main::visuals::yA, 0.f, 5.f);
                }
                ImGui::EndChild();
            }
        }
        ImGui::EndChild();

        ImGui::Columns(1); // 恢复单列布局
        ImGui::PopStyleVar(5); // 恢复所有样式
    }
    ImGui::End();
    ImGui::PopStyleVar(); // 窗口圆角
}
*/