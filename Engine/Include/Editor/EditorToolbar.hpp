#pragma once
#include "Editor/EditorMode.hpp"
#include "Utility/UIUtils.hpp"
#include "imgui.h"
#include "imgui_internal.h"
#include "Renderer/Texture.hpp"
#include <string>
#include <vector>

namespace TE
{

class EditorToolbar
{
public:
    static void OnImGuiRender(const std::shared_ptr<Texture>& saveIcon = nullptr, const std::shared_ptr<Texture>& playIcon = nullptr, const std::shared_ptr<Texture>& brandIcon = nullptr)
    {
        float toolbarHeight = 48.0f;

        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(5, 7));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 8));

        if (ImGui::BeginChild("##ToolbarRegion", ImVec2(0, toolbarHeight), false,
                              ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
        {
            // 0. Branding Icon (Thumbnail)
            if (brandIcon)
            {
                ImGui::SetCursorPos(ImVec2(10, 8));
                ImTextureID brandIconID = (ImTextureID)(uintptr_t)brandIcon->GetRendererID();
                ImGui::Image(brandIconID, ImVec2(32, 32));
                ImGui::SameLine(0, 15);
            }

            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 4.0f);

            // 1. Editor Mode Selector (UE5 style)
            EditorMode *activeMode = EditorModeRegistry::GetActiveMode();
            std::string modeLabel = activeMode ? activeMode->GetName() : "Selection Mode";

            ImGui::SetNextItemWidth(200.0f);
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 4.0f); // Center vertically a bit

            if (ImGui::BeginCombo("##ModeSelector", modeLabel.c_str()))
            {
                for (const auto &mode : EditorModeRegistry::GetModes())
                {
                    bool isSelected = (activeMode == mode.get());
                    if (ImGui::Selectable(mode->GetName(), isSelected))
                    {
                        TE_CORE_INFO("EditorToolbar: Switching to mode '{0}'", mode->GetName());
                        EditorModeRegistry::SetActiveMode(mode->GetName());
                    }
                }
                if (EditorModeRegistry::GetModes().empty())
                {
                    if (ImGui::Selectable("Selection Mode", true))
                    {
                    }
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
            if (saveIcon)
            {
                ImTextureID saveIconID = (ImTextureID)(uintptr_t)saveIcon->GetRendererID();
                if (ImGui::ImageButton("##SaveIconBtn", saveIconID, ImVec2(btnSize, btnSize)))
                {
                    // Placeholder for Save
                }
            }
            else if (ImGui::Button("S", ImVec2(btnSize, btnSize)))
            {
                // Placeholder for Save
            }
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Save Scene");

            ImGui::SameLine();

            // Play/Stop Button
            static bool isPlaying = false;
            if (isPlaying)
            {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
                if (ImGui::Button("Stop", ImVec2(btnSize * 1.5f, btnSize)))
                    isPlaying = false;
                ImGui::PopStyleColor();
            }
            else
            {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2f, 0.7f, 0.2f, 0.4f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.2f, 0.7f, 0.2f, 0.6f));

                if (playIcon)
                {
                    ImTextureID playIconID = (ImTextureID)(uintptr_t)playIcon->GetRendererID();
                    if (ImGui::ImageButton("##PlayIconBtn", playIconID, ImVec2(btnSize * 1.5f - 6, btnSize - 6)))
                        isPlaying = true;
                }
                else
                {
                    if (ImGui::Button("Play", ImVec2(btnSize * 1.5f, btnSize)))
                        isPlaying = true;
                }
                ImGui::PopStyleColor(3);
            }

            ImGui::SameLine(0, 20);
            ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
            ImGui::SameLine(0, 20);
        }
        ImGui::EndChild();
        ImGui::PopStyleVar(2);
    }
};

} // namespace TE
