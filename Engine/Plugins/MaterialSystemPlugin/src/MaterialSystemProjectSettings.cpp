#include "Core/PreRequisites.h"
#include "MaterialSystemProjectSettings.hpp"
#include "Layers/EditorLayer.hpp"
#include "Utils/TimeGUI.hpp"

TE_REGISTER_PROJECT_SETTINGS(MaterialSystemProjectSettings);

namespace
{
    static bool s_EnableShaderCache = true;
    static int s_MaxMaterialCacheMB = 128;
    static bool s_AutoRecompileOnSave = true;
    static int s_TextureCompressionLevel = 1; // 0: None, 1: Fast, 2: High Quality
    static bool s_StrictUniformValidation = false;
}

void MaterialSystemProjectSettings::OnDrawSettingsUI(Ref<EditorLayer> editor)
{
    TimeGUI::TextColored(TEColor(0.2f, 0.7f, 1.0f, 1.0f), "Shader Compilation & Cache");
    TimeGUI::Separator();

    TimeGUI::Checkbox("Enable Shader Cache", &s_EnableShaderCache);
    TimeGUI::Checkbox("Auto-Recompile Shaders On Material Graph Save", &s_AutoRecompileOnSave);
    TimeGUI::SliderInt("Max Material Cache Size", &s_MaxMaterialCacheMB, 32, 1024, "%d MB");

    TimeGUI::Spacing();
    TimeGUI::TextColored(TEColor(0.2f, 0.7f, 1.0f, 1.0f), "Texture & Uniform Policies");
    TimeGUI::Separator();

    const char *compressionPresets[] = { "Uncompressed", "Fast Compression", "High Quality" };
    TimeGUI::Combo("Texture Compression Preset", &s_TextureCompressionLevel, compressionPresets, 3);
    TimeGUI::Checkbox("Strict Uniform Block Validation", &s_StrictUniformValidation);

    TimeGUI::Spacing();
    TimeGUI::Separator();

    if (TimeGUI::Button("Reset Material System Defaults", 230.0f, 26.0f))
    {
        s_EnableShaderCache = true;
        s_MaxMaterialCacheMB = 128;
        s_AutoRecompileOnSave = true;
        s_TextureCompressionLevel = 1;
        s_StrictUniformValidation = false;
    }
}
