#pragma once
#include "Editor/EditorMode.hpp"
#include "Renderer/Texture.hpp"
#include "Utils/TimeGUI.hpp"
#include <string>
#include <vector>

namespace TE
{

class EditorToolbar
{
public:
    static void OnTimeGUIRender(const std::shared_ptr<Texture> &saveIcon = nullptr,
                                const std::shared_ptr<Texture> &playIcon = nullptr,
                                const std::shared_ptr<Texture> &brandIcon = nullptr)
    {
        float toolbarHeight = 48.0f;

        TimeGUI::PushStyleVar(TimeGUI::TimeGUIStyleVar_FramePadding, TEVector2(5, 7));
        TimeGUI::PushStyleVar(TimeGUI::TimeGUIStyleVar_WindowPadding, TEVector2(8, 8));

        if (TimeGUI::BeginChild("##ToolbarRegion", TEVector2(0, toolbarHeight), false,
                                TimeGUI::TimeGUIWindowFlags_NoScrollbar |
                                    TimeGUI::TimeGUIWindowFlags_NoScrollWithMouse))
        {
            // 0. Branding Icon (Thumbnail)
            if (brandIcon)
            {
                TimeGUI::SetCursorPos(TEVector2(10, 8));
                TimeGUI::TimeGUITextureID brandIconID =
                    (TimeGUI::TimeGUITextureID)(uintptr_t)brandIcon->GetRendererID();
                TimeGUI::Image(brandIconID, TEVector2(32, 32));
                TimeGUI::SameLine(0, 15);
            }

            TimeGUI::SetCursorPosY(TimeGUI::GetCursorPosY() + 4.0f);

            // 1. Editor Mode Selector (UE5 style)
            EditorMode *activeMode = EditorModeRegistry::GetActiveMode();
            std::string modeLabel = activeMode ? activeMode->GetName() : "Selection Mode";

            TimeGUI::SetNextItemWidth(200.0f);
            TimeGUI::SetCursorPosY(TimeGUI::GetCursorPosY() + 4.0f); // Center vertically a bit

            if (TimeGUI::BeginCombo("##ModeSelector", modeLabel.c_str()))
            {
                for (const auto &mode : EditorModeRegistry::GetModes())
                {
                    bool isSelected = (activeMode == mode.get());
                    if (TimeGUI::Selectable(mode->GetName(), isSelected))
                    {
                        TE_CORE_INFO("EditorToolbar: Switching to mode '{0}'", mode->GetName());
                        EditorModeRegistry::SetActiveMode(mode->GetName());
                    }
                }
                if (EditorModeRegistry::GetModes().empty())
                {
                    if (TimeGUI::Selectable("Selection Mode", true))
                    {
                    }
                }
                TimeGUI::EndCombo();
            }

            TimeGUI::SameLine(0, 20);
            TimeGUI::SetCursorPosY(TimeGUI::GetCursorPosY() - 4.0f); // Reset
            TimeGUI::SeparatorEx(TimeGUI::TimeGUISeparatorFlags_Vertical);
            TimeGUI::SameLine(0, 20);

            // 2. Core Actions (Save, Play)
            float btnSize = TimeGUI::GetFrameHeight();

            // Save Button
            if (saveIcon)
            {
                TimeGUI::TimeGUITextureID saveIconID = (TimeGUI::TimeGUITextureID)(uintptr_t)saveIcon->GetRendererID();
                if (TimeGUI::ImageButton("##SaveIconBtn", saveIconID, TEVector2(btnSize, btnSize)))
                {
                    // Placeholder for Save
                }
            }
            else if (TimeGUI::Button("S", TEVector2(btnSize, btnSize)))
            {
                // Placeholder for Save
            }
            if (TimeGUI::IsItemHovered())
                TimeGUI::SetTooltip("Save Scene");

            TimeGUI::SameLine();

            // Play/Stop Button
            static bool isPlaying = false;
            if (isPlaying)
            {
                TimeGUI::PushStyleColor(TimeGUI::TimeGUICol_Button, TEColor(0.8f, 0.2f, 0.2f, 1.0f));
                if (TimeGUI::Button("Stop", TEVector2(btnSize * 1.5f, btnSize)))
                    isPlaying = false;
                TimeGUI::PopStyleColor();
            }
            else
            {
                TimeGUI::PushStyleColor(TimeGUI::TimeGUICol_Button, TEColor(0.0f, 0.0f, 0.0f, 0.0f));
                TimeGUI::PushStyleColor(TimeGUI::TimeGUICol_ButtonHovered, TEColor(0.2f, 0.7f, 0.2f, 0.4f));
                TimeGUI::PushStyleColor(TimeGUI::TimeGUICol_ButtonActive, TEColor(0.2f, 0.7f, 0.2f, 0.6f));

                if (playIcon)
                {
                    TimeGUI::TimeGUITextureID playIconID =
                        (TimeGUI::TimeGUITextureID)(uintptr_t)playIcon->GetRendererID();
                    if (TimeGUI::ImageButton("##PlayIconBtn", playIconID, TEVector2(btnSize * 1.5f - 6, btnSize - 6)))
                        isPlaying = true;
                }
                else
                {
                    if (TimeGUI::Button("Play", TEVector2(btnSize * 1.5f, btnSize)))
                        isPlaying = true;
                }
                TimeGUI::PopStyleColor(3);
            }

            TimeGUI::SameLine(0, 20);
            TimeGUI::SeparatorEx(TimeGUI::TimeGUISeparatorFlags_Vertical);
            TimeGUI::SameLine(0, 20);
        }
        TimeGUI::EndChild();
        TimeGUI::PopStyleVar(2);
    }
};

} // namespace TE
