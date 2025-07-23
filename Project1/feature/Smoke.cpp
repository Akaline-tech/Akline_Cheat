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

// ȫ�������洢�����Ŀ�ʼʱ���Ψһ��ʶ��
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

    auto now = std::chrono::steady_clock::now(); // ��ȡ��ǰʱ��
    std::unordered_set<uintptr_t> activeSmokes; // �洢��ǰ��Ч������ʵ��

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
            // �޸����Ƴ�����ķֺ�
            if (!WorldToScreen(m_vSmokeDetonationPos, smokeScreenPos, Matrix, w, h));

            // ��ǵ�ǰ����Ϊ��Ծ״̬
            activeSmokes.insert(C_BaseEntity);

            // �ؼ��޸�����������Ƿ��ѹ���
            if (expiredSmokes.find(C_BaseEntity) != expiredSmokes.end()) {
                continue; // �����ѹ��ڵ�����
            }

            // �ؼ��޸���ʹ����ȷ�ı�־�ж������Ƿ�������
            if (*m_bDidSmokeEffect && main::visuals::Smoketimer) {
                // ������ǵ�һ�μ�⵽�������ɣ���¼��ʼʱ��
                if (smokeStartTimes.find(C_BaseEntity) == smokeStartTimes.end()) {
                    smokeStartTimes[C_BaseEntity] = now;
                }

                // ����ʣ��ʱ��
                auto startTime = smokeStartTimes[C_BaseEntity];
                auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime).count() / 1000.0f;
                float remaining = 20.9f - elapsed;

                // �����ѹ��ڣ������ʱ�������Ϊ�ѹ���
                if (remaining <= 0.0f) {
                    smokeStartTimes.erase(C_BaseEntity);
                    expiredSmokes.insert(C_BaseEntity); // ���Ϊ�ѹ���
                    continue; // ������������
                }
                // ����Բ�μ�ʱ��
                const float radius = 20.0f; // ��ʱ���뾶
                const float thickness = 4.0f; // ���������
                const ImVec2 center(smokeScreenPos.x, smokeScreenPos.y);

                // 1. ���Ʊ���Բ������ɫ��͸����
                ImGui::GetBackgroundDrawList()->AddCircle(
                    center,
                    radius,
                    ImColor(100, 100, 100, 150), // ��ɫ��͸��
                    thickness
                );

                // 2. ����������Ƕȣ���12��λ����ʱ����٣�
                float progress = remaining / 20.0f; // ���ȱ��� (1.0 - 0.0)
                float startAngle = -static_cast<float>(M_PI) / 2.0f; // 12��λ�� (-90��)
                float endAngle = startAngle - 2.0f * static_cast<float>(M_PI) * progress; // ��ʱ�뷽��

                // 3. ���ƽ���������ɫ��͸����
                if (progress > 0.0f) {
                    // ����ʣ��ʱ�������ɫ�����5���죩
                    ImColor progressColor;
                    if (remaining > 5.0f) {
                        progressColor = ImColor(0, 255, 0, 200); // ��ɫ
                    }
                    else {
                        // ʣ��ʱ��Խ�٣���ɫԽ��
                        float intensity = (5.0f - remaining) / 5.0f;
                        progressColor = ImColor(
                            255,
                            static_cast<int>(255 * (1.0f - intensity)),
                            0,
                            200
                        );
                    }

                    // ���ƽ��Ȼ���
                    ImGui::GetBackgroundDrawList()->PathArcTo(
                        center,
                        radius,
                        startAngle,
                        endAngle,
                        32 // �ֶ���
                    );
                    ImGui::GetBackgroundDrawList()->PathStroke(
                        progressColor,
                        false,
                        thickness
                    );
                }

                // 4. ��������ʾʣ������
                if (remaining > 0.1f) {
                    char timeText[8];
                    snprintf(timeText, sizeof(timeText), "%.0f", remaining);

                    // �����ı�λ�ã����У�
                    ImVec2 textSize = ImGui::CalcTextSize(timeText);
                    ImVec2 textPos(center.x - textSize.x / 2, center.y - textSize.y / 2);

                    // ���ư�ɫ�ı�
                    ImGui::GetBackgroundDrawList()->AddText(
                        textPos,
                        ImColor(255, 255, 255, 220),
                        timeText
                    );
                }
            }
            else {
                // �������δ���ɻ�����ʧ���Ӽ�ʱ�����Ƴ�
                if (smokeStartTimes.find(C_BaseEntity) != smokeStartTimes.end()) {
                    smokeStartTimes.erase(C_BaseEntity);
                }
            }

            if (main::visuals::AntiSmoke) {
                *m_nSmokeEffectTickBegin = 0;
            }

            // ��������⻷
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

            // ����������ɫ
            ImVec4 smoke_color = main::visuals::smoke_color;
            if (main::visuals::SmokeColorChanger) {
                *m_vSmokeColor = Vector3(smoke_color.x, smoke_color.y, smoke_color.z);
            }
        }
    }

    // ������ڵļ�ʱ������ֹ�ڴ�й©��
    for (auto it = smokeStartTimes.begin(); it != smokeStartTimes.end();) {
        // ���ʵ���Ƿ���Ȼ��Ծ���ѳ�ʱ
        if (activeSmokes.find(it->first) == activeSmokes.end() ||
            std::chrono::duration_cast<std::chrono::seconds>(now - it->second).count() > 25) {
            it = smokeStartTimes.erase(it);
        }
        else {
            ++it;
        }
    }

    // �����ѹ��ڵ�������¼
    for (auto it = expiredSmokes.begin(); it != expiredSmokes.end();) {
        if (activeSmokes.find(*it) == activeSmokes.end()) {
            it = expiredSmokes.erase(it);
        }
        else {
            ++it;
        }
    }
}