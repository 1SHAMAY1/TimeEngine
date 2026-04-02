#pragma once
#include "Editor/EditorMode.hpp"
#include "imgui.h"
#include "imgui_internal.h"
#include "Utility/UIUtils.hpp"
#include <vector>
#include <string>

namespace TE {

    class EditorToolbar {
    public:
        static void OnImGuiRender() {
            float toolbarHeight = 48.0f;
            
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(5, 7));
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 8));
            
            if (ImGui::BeginChild("##ToolbarRegion", ImVec2(0, toolbarHeight), false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse)) {
                
                // 1. Editor Mode Selector (UE5 style)
                EditorMode* activeMode = EditorModeRegistry::GetActiveMode();
                std::string modeLabel = activeMode ? activeMode->GetName() : "Selection Mode";
                
                ImGui::SetNextItemWidth(200.0f);
                ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 4.0f); // Center vertically a bit
                
                if (ImGui::BeginCombo("##ModeSelector", modeLabel.c_str())) {
                    for (const auto& mode : EditorModeRegistry::GetModes()) {
                        bool isSelected = (activeMode == mode.get());
                        if (ImGui::Selectable(mode->GetName(), isSelected)) {
                            TE_CORE_INFO("EditorToolbar: Switching to mode '{0}'", mode->GetName());
                            EditorModeRegistry::SetActiveMode(mode->GetName());
                        }
                    }
                    if (EditorModeRegistry::GetModes().empty()) {
                         if (ImGui::Selectable("Selection Mode", true)) {}
                    }
                    ImGui::EndCombo();
                }

                ImGui::SameLine(0, 20);
                ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 4.0f); // Reset
                ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
                ImGui::SameLine(0, 20);

                // 2. Core Actions (Save, Play)
                float btnSize = ImGui::GetFrameHeight();
                
                // Save Button
                if (ImGui::Button("S", ImVec2(btnSize, btnSize))) {
                    // Placeholder for Save
                }
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("Save Scene");

                ImGui::SameLine();

                // Play/Stop Button
                static bool isPlaying = false;
                if (isPlaying) {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
                    if (ImGui::Button("Stop", ImVec2(btnSize * 1.5f, btnSize))) isPlaying = false;
                    ImGui::PopStyleColor();
                } else {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.7f, 0.2f, 1.0f));
                    if (ImGui::Button("Play", ImVec2(btnSize * 1.5f, btnSize))) isPlaying = true;
                    ImGui::PopStyleColor();
                }

                ImGui::SameLine(0, 20);
                ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
                ImGui::SameLine(0, 20);
            }
            ImGui::EndChild();
            ImGui::PopStyleVar(2);
        }
    };

}
