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
    // ���ô���Բ�Ǻ͹̶���С
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 8.0f); // ��΢����Բ�ǰ뾶
    ImGui::SetNextWindowSize(ImVec2(1000, 700), ImGuiCond_Always);
    ImGui::Begin(u8"Akline.Tech", nullptr,
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoTitleBar);
    {
        // ���ô����ڱ߾�
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10, 10));
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8, 4));
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10, 10));
        ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 6.0f); // �Ӵ���Բ��

        // ʹ�����в��֣���ఴť�����Ҳ�������
        ImGui::Columns(2, "tabs", false);
        ImGui::SetColumnWidth(0, 150.0f); // �̶�����п�

        // ��ఴť����
        static int selected_tab = 0; // 0=�Ӿ�, 1=ѹǹ
        ImGui::BeginChild("##Buttons", ImVec2(0, 0), true, ImGuiWindowFlags_AlwaysUseWindowPadding);
        {
            // ��ť��ʽ��ѡ��״̬����
            ImVec2 buttonSize(-FLT_MIN, 50);

            ImGui::PushStyleColor(ImGuiCol_Button, selected_tab == 0 ?
                ImGui::GetStyle().Colors[ImGuiCol_ButtonActive] :
                ImGui::GetStyle().Colors[ImGuiCol_Button]);
            if (ImGui::Button(u8"�Ӿ�����", buttonSize)) selected_tab = 0;
            ImGui::PopStyleColor();

            ImGui::PushStyleColor(ImGuiCol_Button, selected_tab == 1 ?
                ImGui::GetStyle().Colors[ImGuiCol_ButtonActive] :
                ImGui::GetStyle().Colors[ImGuiCol_Button]);
            if (ImGui::Button(u8"ѹǹ����", buttonSize)) selected_tab = 1;
            ImGui::PopStyleColor();
        }
        ImGui::EndChild();

        ImGui::NextColumn(); // �л����Ҳ�������

        // �Ҳ��������� - ʹ�ù̶��߶ȱ��⼷ѹ
        ImGui::BeginChild("##Content", ImVec2(0, ImGui::GetContentRegionAvail().y), true, ImGuiWindowFlags_AlwaysUseWindowPadding);
        {
            if (selected_tab == 0) { // �Ӿ�ҳ��
                // ������ø߶Ȳ���ȥ���
                float content_height = ImGui::GetContentRegionAvail().y - ImGui::GetStyle().ItemSpacing.y;

                // ��ࣺESP���÷���� (60%���)
                ImGui::BeginChild("##ESPSettings", ImVec2(ImGui::GetContentRegionAvail().x * 0.6f, content_height), true);
                {
                    // ��������
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.2f, 0.6f, 1.0f, 1.0f));
                    ImGui::SetCursorPos(ImVec2(15, 8));
                    ImGui::Text(u8"ESP����");
                    ImGui::PopStyleColor();
                    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 15);

                    // ESP���ÿؼ�
                    ImGui::Checkbox(u8"����͸��", &main::visuals::box);
                    ImGui::SameLine();
                    ImGui::SetNextItemWidth(150);
                    ImGui::SliderFloat(u8"�����ϸ##1", &main::visuals::Box_Thickness, 1.0f, 5.0f);

                    ImGui::Checkbox(u8"��䷽��", &main::visuals::boxFill);
                    ImGui::Checkbox(u8"��������", &main::visuals::bone);
                    ImGui::Checkbox(u8"��ʾ����", &main::visuals::name);
                    ImGui::Checkbox(u8"װ��״̬", &main::visuals::Reload);

                    ImGui::Checkbox(u8"ֱ��ָʾ", &main::visuals::Line);
                    ImGui::SameLine();
                    ImGui::SetNextItemWidth(150);
                    ImGui::SliderFloat(u8"ֱ�ߴ�ϸ##2", &main::visuals::Line_Thickness, 1.0f, 5.0f);

                    ImGui::Checkbox(u8"��ɫ�⻷", &main::visuals::halo);
                    ImGui::SameLine();
                    ImGui::SetNextItemWidth(150);
                    ImGui::SliderFloat(u8"�⻷�뾶", &main::visuals::radius, 5.0f, 250.0f);
                    ImGui::SameLine();
                    ImGui::SetNextItemWidth(150);
                    ImGui::SliderInt(u8"�⻷����", &main::visuals::segments, 12, 64);

                    ImGui::Checkbox(u8"ͷ�����", &main::visuals::head);
                    ImGui::Checkbox(u8"��ʾѪ��", &main::visuals::health);
                    ImGui::Checkbox(u8"�������", &main::visuals::team);
                    ImGui::Checkbox(u8"��������", &main::visuals::AntiSmoke);
                    ImGui::Checkbox(u8"����⻷", &main::visuals::SmokeHalo);
                    ImGui::Checkbox(u8"������ɫ�����", &main::visuals::SmokeColorChanger);
                }
                ImGui::EndChild();

                ImGui::SameLine(); // ȷ���Ҳ����ͬһ��

                // �Ҳࣺ��ɫ��������� (40%���)
                ImGui::BeginChild("##ColorSettings", ImVec2(0, content_height), true);
                {
                    // ��������
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.2f, 0.6f, 1.0f, 1.0f));
                    ImGui::SetCursorPos(ImVec2(15, 8));
                    ImGui::Text(u8"��ɫ����");
                    ImGui::PopStyleColor();
                    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 15);


                    ImGui::ColorEdit4(u8"�߿���ɫ", (float*)&main::visuals::box_color, ImGuiColorEditFlags_NoInputs);
                    ImGui::ColorEdit4(u8"�����ɫ", (float*)&main::visuals::Fill_color, ImGuiColorEditFlags_NoInputs);
                    ImGui::ColorEdit4(u8"�⻷��ɫ", (float*)&main::visuals::halo_color, ImGuiColorEditFlags_NoInputs);
                    ImGui::ColorEdit3(u8"������ɫ", (float*)&main::visuals::smoke_color, ImGuiColorEditFlags_NoInputs);
                    ImGui::ColorEdit4(u8"����⻷��ɫ", (float*)&main::visuals::smoke_halo_color, ImGuiColorEditFlags_NoInputs);
                    ImGui::Dummy(ImVec2(0, 20));
                }
                ImGui::EndChild();
            }
            else if (selected_tab == 1) { // ѹǹҳ��
                // ѹǹ���÷����
                ImGui::BeginChild("##AimbotSettings", ImVec2(0, 0), true);
                {
                    // ��������
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.2f, 0.6f, 1.0f, 1.0f));
                    ImGui::SetCursorPos(ImVec2(15, 8));
                    ImGui::Text(u8"ѹǹ����");
                    ImGui::PopStyleColor();
                    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 15);

                    ImGui::Text(u8"ѹǹ��������");
                    ImGui::Separator();
                    ImGui::Checkbox(u8"�Զ�ѹǹ", &main::rcs::rcs);
                    ImGui::SameLine();
                    ImGui::SetNextItemWidth(200);
                    ImGui::SliderInt(u8"�ڼ�����ѹǹ", &main::rcs::start_ammor, 0, 30);
                    ImGui::SliderFloat(u8"ѹǹx", &main::visuals::xA, 0.f, 5.f);
                    ImGui::SliderFloat(u8"ѹǹy", &main::visuals::yA, 0.f, 5.f);
                }
                ImGui::EndChild();
            }
        }
        ImGui::EndChild();

        ImGui::Columns(1); // �ָ����в���
        ImGui::PopStyleVar(5); // �ָ�������ʽ
    }
    ImGui::End();
    ImGui::PopStyleVar(); // ����Բ��
}
*/