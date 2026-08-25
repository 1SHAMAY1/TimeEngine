#include "Core/PreRequisites.h"
#include "Editor/Settings/EditorAppearanceSettings.hpp"
#include "Layers/EditorLayer.hpp"
#include "Utils/TimeGUI.hpp"

TE_REGISTER_EDITOR_SETTINGS(EditorAppearanceSettings);

namespace
{
    static float s_UIScaling = 1.0f;
    static float s_FontSize = 14.0f;
    static float s_AccentColor[4] = { 0.2f, 0.55f, 0.9f, 1.0f };
    static bool s_ConsoleAutoScroll = true;
    static bool s_HighlightActiveTab = true;
    static bool s_ShowAssetExtensions = true;
}

void EditorAppearanceSettings::OnDrawSettingsUI(Ref<EditorLayer> editor)
{
    TimeGUI::TextColored(TEColor(0.2f, 0.7f, 1.0f, 1.0f), "Interface & Theme");
    TimeGUI::Separator();

    TimeGUI::SliderFloat("UI Scaling Factor", &s_UIScaling, 0.75f, 2.5f, "%.2fx");
    TimeGUI::SliderFloat("Base Font Size", &s_FontSize, 10.0f, 24.0f, "%.0f px");
    TimeGUI::ColorEdit4("Accent / Selection Color", s_AccentColor);

    TimeGUI::Spacing();
    TimeGUI::TextColored(TEColor(0.2f, 0.7f, 1.0f, 1.0f), "Editor Panel Behaviors");
    TimeGUI::Separator();

    TimeGUI::Checkbox("Console Auto-Scroll to Latest Log", &s_ConsoleAutoScroll);
    TimeGUI::Checkbox("Highlight Active Docked Tab", &s_HighlightActiveTab);
    TimeGUI::Checkbox("Show File Extensions in Content Browser", &s_ShowAssetExtensions);

    TimeGUI::Spacing();
    TimeGUI::Separator();

    if (TimeGUI::Button("Reset Appearance Defaults", 200.0f, 26.0f))
    {
        s_UIScaling = 1.0f;
        s_FontSize = 14.0f;
        s_AccentColor[0] = 0.2f;
        s_AccentColor[1] = 0.55f;
        s_AccentColor[2] = 0.9f;
        s_AccentColor[3] = 1.0f;
        s_ConsoleAutoScroll = true;
        s_HighlightActiveTab = true;
        s_ShowAssetExtensions = true;
    }
}
