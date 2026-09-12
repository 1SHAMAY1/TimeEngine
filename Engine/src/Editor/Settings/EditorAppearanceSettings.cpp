#include "Core/PreRequisites.h"
#include "Editor/Settings/EditorAppearanceSettings.hpp"
#include "Editor/EditorUtils.hpp"
#include "Layers/EditorLayer.hpp"
#include "Utils/TimeGUI.hpp"

TE_REGISTER_EDITOR_SETTINGS(EditorAppearanceSettings);

namespace
{
static int s_SelectedThemePreset = 0;
static const char *s_ThemePresets[] = {"Showcase Pro Dark (Default)", "Charcoal Studio", "Studio Slate",
                                       "Midnight Blue Glass",         "Nordic Frost",    "Custom"};
constexpr int s_PresetCount = sizeof(s_ThemePresets) / sizeof(s_ThemePresets[0]);

static float s_UIScaling = 1.0f;
static float s_FontSize = 14.0f;
static bool s_ConsoleAutoScroll = true;
static bool s_HighlightActiveTab = true;
static bool s_ShowAssetExtensions = true;

// Helper to apply preset
static void ApplyPreset(int presetIdx)
{
    auto &style = TimeGUI::GetStyle();
    auto &colors = style.Colors;

    s_SelectedThemePreset = presetIdx;

    if (presetIdx == 0) // Showcase Pro Dark
    {
        EditorUtils::SetEditorThemeColors();
    }
    else if (presetIdx == 1) // Charcoal Studio
    {
        colors[TimeGUICol_WindowBg] = TEColor(0.10f, 0.105f, 0.11f, 1.0f);
        colors[TimeGUICol_ChildBg] = TEColor(0.12f, 0.125f, 0.135f, 0.95f);
        colors[TimeGUICol_PopupBg] = TEColor(0.10f, 0.105f, 0.11f, 1.0f);
        colors[TimeGUICol_MenuBarBg] = TEColor(0.08f, 0.085f, 0.095f, 1.0f);
        colors[TimeGUICol_FrameBg] = TEColor(0.15f, 0.16f, 0.18f, 0.85f);
        colors[TimeGUICol_FrameBgHovered] = TEColor(0.20f, 0.22f, 0.25f, 1.0f);
        colors[TimeGUICol_FrameBgActive] = TEColor(0.17f, 0.18f, 0.20f, 1.0f);
        colors[TimeGUICol_Tab] = TEColor(0.12f, 0.13f, 0.14f, 1.0f);
        colors[TimeGUICol_TabHovered] = TEColor(0.18f, 0.20f, 0.23f, 1.0f);
        colors[TimeGUICol_TabSelected] = TEColor(0.15f, 0.16f, 0.18f, 1.0f);
        colors[TimeGUICol_TabSelectedOverline] = TEColor(0.35f, 0.60f, 0.85f, 1.0f);
        colors[TimeGUICol_Button] = TEColor(0.16f, 0.18f, 0.20f, 0.85f);
        colors[TimeGUICol_ButtonHovered] = TEColor(0.24f, 0.28f, 0.34f, 1.0f);
        colors[TimeGUICol_ButtonActive] = TEColor(0.18f, 0.20f, 0.24f, 1.0f);
        colors[TimeGUICol_Header] = TEColor(0.17f, 0.19f, 0.22f, 0.70f);
        colors[TimeGUICol_HeaderHovered] = TEColor(0.24f, 0.28f, 0.34f, 0.85f);
        colors[TimeGUICol_HeaderActive] = TEColor(0.18f, 0.20f, 0.24f, 1.0f);
        colors[TimeGUICol_CheckMark] = TEColor(1.0f, 1.0f, 1.0f, 1.0f);
        colors[TimeGUICol_SliderGrab] = TEColor(0.35f, 0.60f, 0.85f, 0.9f);
        colors[TimeGUICol_SliderGrabActive] = TEColor(0.45f, 0.70f, 0.95f, 1.0f);
        colors[TimeGUICol_Border] = TEColor(0.18f, 0.19f, 0.22f, 0.75f);
        colors[TimeGUICol_Separator] = TEColor(0.16f, 0.18f, 0.20f, 0.80f);
    }
    else if (presetIdx == 2) // Studio Slate
    {
        colors[TimeGUICol_WindowBg] = TEColor(0.13f, 0.14f, 0.16f, 1.0f);
        colors[TimeGUICol_ChildBg] = TEColor(0.15f, 0.16f, 0.18f, 0.95f);
        colors[TimeGUICol_PopupBg] = TEColor(0.13f, 0.14f, 0.16f, 1.0f);
        colors[TimeGUICol_MenuBarBg] = TEColor(0.11f, 0.12f, 0.14f, 1.0f);
        colors[TimeGUICol_FrameBg] = TEColor(0.18f, 0.20f, 0.23f, 0.85f);
        colors[TimeGUICol_FrameBgHovered] = TEColor(0.24f, 0.27f, 0.32f, 1.0f);
        colors[TimeGUICol_FrameBgActive] = TEColor(0.20f, 0.22f, 0.26f, 1.0f);
        colors[TimeGUICol_Tab] = TEColor(0.15f, 0.16f, 0.18f, 1.0f);
        colors[TimeGUICol_TabHovered] = TEColor(0.24f, 0.26f, 0.30f, 1.0f);
        colors[TimeGUICol_TabSelected] = TEColor(0.19f, 0.21f, 0.24f, 1.0f);
        colors[TimeGUICol_TabSelectedOverline] = TEColor(0.90f, 0.55f, 0.15f, 1.0f);
        colors[TimeGUICol_Button] = TEColor(0.20f, 0.23f, 0.27f, 0.85f);
        colors[TimeGUICol_ButtonHovered] = TEColor(0.28f, 0.33f, 0.40f, 1.0f);
        colors[TimeGUICol_ButtonActive] = TEColor(0.24f, 0.27f, 0.32f, 1.0f);
        colors[TimeGUICol_Header] = TEColor(0.22f, 0.25f, 0.30f, 0.70f);
        colors[TimeGUICol_HeaderHovered] = TEColor(0.28f, 0.33f, 0.40f, 0.85f);
        colors[TimeGUICol_HeaderActive] = TEColor(0.24f, 0.27f, 0.32f, 1.0f);
        colors[TimeGUICol_CheckMark] = TEColor(1.0f, 1.0f, 1.0f, 1.0f);
        colors[TimeGUICol_SliderGrab] = TEColor(0.90f, 0.55f, 0.15f, 0.9f);
        colors[TimeGUICol_SliderGrabActive] = TEColor(1.0f, 0.65f, 0.25f, 1.0f);
        colors[TimeGUICol_Border] = TEColor(0.24f, 0.26f, 0.30f, 0.75f);
        colors[TimeGUICol_Separator] = TEColor(0.22f, 0.24f, 0.28f, 0.80f);
    }
    else if (presetIdx == 3) // Midnight Blue Glass
    {
        colors[TimeGUICol_WindowBg] = TEColor(0.09f, 0.11f, 0.15f, 1.0f);
        colors[TimeGUICol_ChildBg] = TEColor(0.11f, 0.13f, 0.18f, 0.85f);
        colors[TimeGUICol_PopupBg] = TEColor(0.09f, 0.11f, 0.15f, 1.0f);
        colors[TimeGUICol_MenuBarBg] = TEColor(0.07f, 0.09f, 0.12f, 1.0f);
        colors[TimeGUICol_FrameBg] = TEColor(0.14f, 0.18f, 0.24f, 0.85f);
        colors[TimeGUICol_FrameBgHovered] = TEColor(0.20f, 0.25f, 0.34f, 1.0f);
        colors[TimeGUICol_FrameBgActive] = TEColor(0.16f, 0.20f, 0.28f, 1.0f);
        colors[TimeGUICol_Tab] = TEColor(0.11f, 0.14f, 0.18f, 1.0f);
        colors[TimeGUICol_TabHovered] = TEColor(0.18f, 0.24f, 0.32f, 1.0f);
        colors[TimeGUICol_TabSelected] = TEColor(0.14f, 0.18f, 0.24f, 1.0f);
        colors[TimeGUICol_TabSelectedOverline] = TEColor(0.20f, 0.65f, 0.95f, 1.0f);
        colors[TimeGUICol_Button] = TEColor(0.15f, 0.20f, 0.28f, 0.85f);
        colors[TimeGUICol_ButtonHovered] = TEColor(0.24f, 0.32f, 0.44f, 1.0f);
        colors[TimeGUICol_ButtonActive] = TEColor(0.18f, 0.23f, 0.32f, 1.0f);
        colors[TimeGUICol_Header] = TEColor(0.16f, 0.22f, 0.30f, 0.70f);
        colors[TimeGUICol_HeaderHovered] = TEColor(0.24f, 0.32f, 0.44f, 0.85f);
        colors[TimeGUICol_HeaderActive] = TEColor(0.18f, 0.24f, 0.32f, 1.0f);
        colors[TimeGUICol_CheckMark] = TEColor(1.0f, 1.0f, 1.0f, 1.0f);
        colors[TimeGUICol_SliderGrab] = TEColor(0.20f, 0.65f, 0.95f, 0.9f);
        colors[TimeGUICol_SliderGrabActive] = TEColor(0.30f, 0.75f, 1.0f, 1.0f);
        colors[TimeGUICol_Border] = TEColor(0.18f, 0.24f, 0.32f, 0.75f);
        colors[TimeGUICol_Separator] = TEColor(0.16f, 0.20f, 0.28f, 0.80f);
    }
    else if (presetIdx == 4) // Nordic Frost
    {
        colors[TimeGUICol_WindowBg] = TEColor(0.15f, 0.17f, 0.20f, 1.0f);
        colors[TimeGUICol_ChildBg] = TEColor(0.18f, 0.20f, 0.24f, 0.95f);
        colors[TimeGUICol_PopupBg] = TEColor(0.15f, 0.17f, 0.20f, 1.0f);
        colors[TimeGUICol_MenuBarBg] = TEColor(0.12f, 0.14f, 0.17f, 1.0f);
        colors[TimeGUICol_FrameBg] = TEColor(0.22f, 0.25f, 0.30f, 0.85f);
        colors[TimeGUICol_FrameBgHovered] = TEColor(0.28f, 0.32f, 0.38f, 1.0f);
        colors[TimeGUICol_FrameBgActive] = TEColor(0.24f, 0.28f, 0.34f, 1.0f);
        colors[TimeGUICol_Tab] = TEColor(0.18f, 0.20f, 0.24f, 1.0f);
        colors[TimeGUICol_TabHovered] = TEColor(0.26f, 0.30f, 0.36f, 1.0f);
        colors[TimeGUICol_TabSelected] = TEColor(0.22f, 0.25f, 0.30f, 1.0f);
        colors[TimeGUICol_TabSelectedOverline] = TEColor(0.53f, 0.75f, 0.82f, 1.0f);
        colors[TimeGUICol_Button] = TEColor(0.22f, 0.26f, 0.32f, 0.85f);
        colors[TimeGUICol_ButtonHovered] = TEColor(0.32f, 0.38f, 0.46f, 1.0f);
        colors[TimeGUICol_ButtonActive] = TEColor(0.25f, 0.30f, 0.36f, 1.0f);
        colors[TimeGUICol_Header] = TEColor(0.24f, 0.28f, 0.34f, 0.70f);
        colors[TimeGUICol_HeaderHovered] = TEColor(0.32f, 0.38f, 0.46f, 0.85f);
        colors[TimeGUICol_HeaderActive] = TEColor(0.26f, 0.30f, 0.36f, 1.0f);
        colors[TimeGUICol_CheckMark] = TEColor(1.0f, 1.0f, 1.0f, 1.0f);
        colors[TimeGUICol_SliderGrab] = TEColor(0.53f, 0.75f, 0.82f, 0.9f);
        colors[TimeGUICol_SliderGrabActive] = TEColor(0.65f, 0.85f, 0.92f, 1.0f);
        colors[TimeGUICol_Border] = TEColor(0.26f, 0.30f, 0.36f, 0.75f);
        colors[TimeGUICol_Separator] = TEColor(0.22f, 0.26f, 0.32f, 0.80f);
    }
}
} // namespace

void EditorAppearanceSettings::OnDrawSettingsUI(Ref<EditorLayer> editor)
{
    auto &style = TimeGUI::GetStyle();
    auto &colors = style.Colors;

    TimeGUI::TextColored(TEColor(0.2f, 0.7f, 1.0f, 1.0f), "Theme & Color Palette");
    TimeGUI::Separator();
    TimeGUI::Spacing();

    // Theme Preset Selector
    TimeGUI::TextColored(TEColor(0.85f, 0.85f, 0.9f, 1.0f), "Theme Preset:");
    TimeGUI::SetNextItemWidth(360.0f);
    if (TimeGUI::Combo("##ActiveThemePreset", &s_SelectedThemePreset, s_ThemePresets, s_PresetCount))
    {
        if (s_SelectedThemePreset < s_PresetCount - 1)
        {
            ApplyPreset(s_SelectedThemePreset);
        }
    }

    TimeGUI::Spacing();
    TimeGUI::Separator();
    TimeGUI::Spacing();

    // --- Section 1: Surface & Background Colors ---
    TimeGUI::TextColored(TEColor(0.85f, 0.85f, 0.9f, 1.0f), "Surface & Background Colors (With Transparency Alpha)");
    TimeGUI::Spacing();

    float windowBg[4] = {colors[TimeGUICol_WindowBg].r, colors[TimeGUICol_WindowBg].g, colors[TimeGUICol_WindowBg].b,
                         colors[TimeGUICol_WindowBg].a};
    if (TimeGUI::ColorEdit4("Window Background (Alpha = Translucency)", windowBg))
    {
        colors[TimeGUICol_WindowBg] = TEColor(windowBg[0], windowBg[1], windowBg[2], windowBg[3]);
        s_SelectedThemePreset = s_PresetCount - 1; // Custom
    }

    float childBg[4] = {colors[TimeGUICol_ChildBg].r, colors[TimeGUICol_ChildBg].g, colors[TimeGUICol_ChildBg].b,
                        colors[TimeGUICol_ChildBg].a};
    if (TimeGUI::ColorEdit4("Panel / Child Background", childBg))
    {
        colors[TimeGUICol_ChildBg] = TEColor(childBg[0], childBg[1], childBg[2], childBg[3]);
        s_SelectedThemePreset = s_PresetCount - 1;
    }

    float popupBg[4] = {colors[TimeGUICol_PopupBg].r, colors[TimeGUICol_PopupBg].g, colors[TimeGUICol_PopupBg].b,
                        colors[TimeGUICol_PopupBg].a};
    if (TimeGUI::ColorEdit4("Popup / Context Menu Background", popupBg))
    {
        colors[TimeGUICol_PopupBg] = TEColor(popupBg[0], popupBg[1], popupBg[2], popupBg[3]);
        s_SelectedThemePreset = s_PresetCount - 1;
    }

    float menuBarBg[4] = {colors[TimeGUICol_MenuBarBg].r, colors[TimeGUICol_MenuBarBg].g,
                          colors[TimeGUICol_MenuBarBg].b, colors[TimeGUICol_MenuBarBg].a};
    if (TimeGUI::ColorEdit4("Menu Bar Background", menuBarBg))
    {
        colors[TimeGUICol_MenuBarBg] = TEColor(menuBarBg[0], menuBarBg[1], menuBarBg[2], menuBarBg[3]);
        s_SelectedThemePreset = s_PresetCount - 1;
    }

    float frameBg[4] = {colors[TimeGUICol_FrameBg].r, colors[TimeGUICol_FrameBg].g, colors[TimeGUICol_FrameBg].b,
                        colors[TimeGUICol_FrameBg].a};
    if (TimeGUI::ColorEdit4("Frame / Input Field Background", frameBg))
    {
        colors[TimeGUICol_FrameBg] = TEColor(frameBg[0], frameBg[1], frameBg[2], frameBg[3]);
        s_SelectedThemePreset = s_PresetCount - 1;
    }

    TimeGUI::Spacing();
    TimeGUI::Separator();
    TimeGUI::Spacing();

    // --- Section 2: Interactive Elements & Accents ---
    TimeGUI::TextColored(TEColor(0.85f, 0.85f, 0.9f, 1.0f), "Interactive Elements & Accents");
    TimeGUI::Spacing();

    float accentColor[4] = {colors[TimeGUICol_SliderGrab].r, colors[TimeGUICol_SliderGrab].g,
                            colors[TimeGUICol_SliderGrab].b, colors[TimeGUICol_SliderGrab].a};
    if (TimeGUI::ColorEdit4("Primary Accent (Sliders / Highlights)", accentColor))
    {
        colors[TimeGUICol_SliderGrab] = TEColor(accentColor[0], accentColor[1], accentColor[2], accentColor[3] * 0.9f);
        colors[TimeGUICol_SliderGrabActive] = TEColor(accentColor[0], accentColor[1], accentColor[2], 1.0f);
        colors[TimeGUICol_TabSelectedOverline] = TEColor(accentColor[0], accentColor[1], accentColor[2], 1.0f);
        colors[TimeGUICol_DockingPreview] = TEColor(accentColor[0], accentColor[1], accentColor[2], 0.4f);
        s_SelectedThemePreset = s_PresetCount - 1;
    }

    float checkColor[4] = {colors[TimeGUICol_CheckMark].r, colors[TimeGUICol_CheckMark].g,
                           colors[TimeGUICol_CheckMark].b, colors[TimeGUICol_CheckMark].a};
    if (TimeGUI::ColorEdit4("Checkbox Tick Mark Color", checkColor))
    {
        colors[TimeGUICol_CheckMark] = TEColor(checkColor[0], checkColor[1], checkColor[2], checkColor[3]);
        s_SelectedThemePreset = s_PresetCount - 1;
    }

    float buttonColor[4] = {colors[TimeGUICol_Button].r, colors[TimeGUICol_Button].g, colors[TimeGUICol_Button].b,
                            colors[TimeGUICol_Button].a};
    if (TimeGUI::ColorEdit4("Button Normal", buttonColor))
    {
        colors[TimeGUICol_Button] = TEColor(buttonColor[0], buttonColor[1], buttonColor[2], buttonColor[3]);
        s_SelectedThemePreset = s_PresetCount - 1;
    }

    float buttonHovered[4] = {colors[TimeGUICol_ButtonHovered].r, colors[TimeGUICol_ButtonHovered].g,
                              colors[TimeGUICol_ButtonHovered].b, colors[TimeGUICol_ButtonHovered].a};
    if (TimeGUI::ColorEdit4("Button Hovered", buttonHovered))
    {
        colors[TimeGUICol_ButtonHovered] =
            TEColor(buttonHovered[0], buttonHovered[1], buttonHovered[2], buttonHovered[3]);
        s_SelectedThemePreset = s_PresetCount - 1;
    }

    float tabSelectedColor[4] = {colors[TimeGUICol_TabSelected].r, colors[TimeGUICol_TabSelected].g,
                                 colors[TimeGUICol_TabSelected].b, colors[TimeGUICol_TabSelected].a};
    if (TimeGUI::ColorEdit4("Selected Docked Window / Tab Background (Set Alpha for Translucency)", tabSelectedColor))
    {
        colors[TimeGUICol_TabSelected] =
            TEColor(tabSelectedColor[0], tabSelectedColor[1], tabSelectedColor[2], tabSelectedColor[3]);
        s_SelectedThemePreset = s_PresetCount - 1;
    }

    float tabDimmedSelected[4] = {colors[TimeGUICol_TabDimmedSelected].r, colors[TimeGUICol_TabDimmedSelected].g,
                                  colors[TimeGUICol_TabDimmedSelected].b, colors[TimeGUICol_TabDimmedSelected].a};
    if (TimeGUI::ColorEdit4("Inactive Dock Tab Selected Background", tabDimmedSelected))
    {
        colors[TimeGUICol_TabDimmedSelected] =
            TEColor(tabDimmedSelected[0], tabDimmedSelected[1], tabDimmedSelected[2], tabDimmedSelected[3]);
        s_SelectedThemePreset = s_PresetCount - 1;
    }

    float headerColor[4] = {colors[TimeGUICol_Header].r, colors[TimeGUICol_Header].g, colors[TimeGUICol_Header].b,
                            colors[TimeGUICol_Header].a};
    if (TimeGUI::ColorEdit4("Header / Selectable Highlight", headerColor))
    {
        colors[TimeGUICol_Header] = TEColor(headerColor[0], headerColor[1], headerColor[2], headerColor[3]);
        s_SelectedThemePreset = s_PresetCount - 1;
    }

    TimeGUI::Spacing();
    TimeGUI::Separator();
    TimeGUI::Spacing();

    // --- Section 3: Text & Borders ---
    TimeGUI::TextColored(TEColor(0.85f, 0.85f, 0.9f, 1.0f), "Typography & Borders");
    TimeGUI::Spacing();

    float textColor[4] = {colors[TimeGUICol_Text].r, colors[TimeGUICol_Text].g, colors[TimeGUICol_Text].b,
                          colors[TimeGUICol_Text].a};
    if (TimeGUI::ColorEdit4("Text Primary", textColor))
    {
        colors[TimeGUICol_Text] = TEColor(textColor[0], textColor[1], textColor[2], textColor[3]);
        s_SelectedThemePreset = s_PresetCount - 1;
    }

    float textDisabled[4] = {colors[TimeGUICol_TextDisabled].r, colors[TimeGUICol_TextDisabled].g,
                             colors[TimeGUICol_TextDisabled].b, colors[TimeGUICol_TextDisabled].a};
    if (TimeGUI::ColorEdit4("Text Muted / Disabled", textDisabled))
    {
        colors[TimeGUICol_TextDisabled] = TEColor(textDisabled[0], textDisabled[1], textDisabled[2], textDisabled[3]);
        s_SelectedThemePreset = s_PresetCount - 1;
    }

    float borderColor[4] = {colors[TimeGUICol_Border].r, colors[TimeGUICol_Border].g, colors[TimeGUICol_Border].b,
                            colors[TimeGUICol_Border].a};
    if (TimeGUI::ColorEdit4("Border Color", borderColor))
    {
        colors[TimeGUICol_Border] = TEColor(borderColor[0], borderColor[1], borderColor[2], borderColor[3]);
        s_SelectedThemePreset = s_PresetCount - 1;
    }

    TimeGUI::Spacing();
    TimeGUI::Separator();
    TimeGUI::Spacing();

    // --- Section 4: Geometry & Rounding ---
    TimeGUI::TextColored(TEColor(0.85f, 0.85f, 0.9f, 1.0f), "Geometry & Rounding");
    TimeGUI::Spacing();

    TimeGUI::SliderFloat("Window Rounding", &style.WindowRounding, 0.0f, 16.0f, "%.1f px");
    TimeGUI::SliderFloat("Frame Rounding", &style.FrameRounding, 0.0f, 12.0f, "%.1f px");
    TimeGUI::SliderFloat("Tab Rounding", &style.TabRounding, 0.0f, 12.0f, "%.1f px");
    TimeGUI::SliderFloat("Child Panel Rounding", &style.ChildRounding, 0.0f, 12.0f, "%.1f px");
    TimeGUI::SliderFloat("Frame Border Width", &style.FrameBorderSize, 0.0f, 3.0f, "%.1f px");

    TimeGUI::Spacing();
    TimeGUI::Separator();
    TimeGUI::Spacing();

    // --- Section 5: Panel Behaviors ---
    TimeGUI::TextColored(TEColor(0.85f, 0.85f, 0.9f, 1.0f), "Editor Panel Behaviors");
    TimeGUI::Spacing();

    TimeGUI::Checkbox("Console Auto-Scroll to Latest Log", &s_ConsoleAutoScroll);
    TimeGUI::Checkbox("Highlight Active Docked Tab", &s_HighlightActiveTab);
    TimeGUI::Checkbox("Show File Extensions in Content Browser", &s_ShowAssetExtensions);

    TimeGUI::Spacing();
    TimeGUI::Separator();
    TimeGUI::Spacing();

    if (TimeGUI::Button("Reset to Showcase Defaults", 220.0f, 28.0f))
    {
        ApplyPreset(0);
        s_UIScaling = 1.0f;
        s_FontSize = 14.0f;
        s_ConsoleAutoScroll = true;
        s_HighlightActiveTab = true;
        s_ShowAssetExtensions = true;
    }
}
