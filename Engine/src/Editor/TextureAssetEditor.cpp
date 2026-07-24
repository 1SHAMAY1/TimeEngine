#include "Editor/TextureAssetEditor.hpp"

#include "Core/Log.h"
#include "Editor/AssetEditorRegistry.hpp"
#include "Renderer/Texture.hpp"
#include "Renderer/TextureSerializer.hpp"
#include "Utils/PlatformUtils.hpp"
#include "Utils/TimeGUI.hpp"
#include <filesystem>
#include <string>

namespace TE
{

void TextureAssetEditor::DrawEditor(EditorTab &tab)
{
    auto tex = std::dynamic_pointer_cast<Texture>(tab.LoadedAsset);
    if (!tex) return;

    static bool s_ChannelR = true;
    static bool s_ChannelG = true;
    static bool s_ChannelB = true;
    static bool s_ChannelA = true;
    static float s_CanvasZoom = 1.0f;
    static float s_UVTiling = 1.0f;

    TimeGUI::Columns(2, "TextureEditorMainSplitter", true);

    // --- LEFT PANEL: Settings & Metadata ---
    TimeGUI::BeginChild("TextureLeftPanel", TEVector2(0.0f, 0.0f), false);

    TimeGUI::Text("Texture Settings & Metadata");
    TimeGUI::TextDisabled("Name: %s", tex->GetName().c_str());
    TimeGUI::TextDisabled("Resolution: %u x %u", tex->GetWidth(), tex->GetHeight());
    TimeGUI::TextDisabled("Channels: %u", tex->GetChannels());

    TimeGUI::Separator();
    TimeGUI::Text("Sampling Settings");

    int filterIdx = (tex->GetFilterMode() == TextureFilterMode::Nearest) ? 1 : 0;
    const char *filterItems[] = {"Linear (Bilinear)", "Nearest (Point / Pixel Art)"};
    if (TimeGUI::Combo("Filter Mode", &filterIdx, filterItems, 2))
    {
        tex->SetFilterMode((filterIdx == 1) ? TextureFilterMode::Nearest : TextureFilterMode::Linear);
        TextureSerializer serializer(tex);
        serializer.Serialize(tab.AssetPath);
    }

    int wrapIdx = 0;
    if (tex->GetWrapMode() == TextureWrapMode::ClampToEdge)
        wrapIdx = 1;
    else if (tex->GetWrapMode() == TextureWrapMode::MirroredRepeat)
        wrapIdx = 2;

    const char *wrapItems[] = {"Repeat", "Clamp To Edge", "Mirrored Repeat"};
    if (TimeGUI::Combo("Wrap Mode", &wrapIdx, wrapItems, 3))
    {
        TextureWrapMode newWrap = TextureWrapMode::Repeat;
        if (wrapIdx == 1)
            newWrap = TextureWrapMode::ClampToEdge;
        else if (wrapIdx == 2)
            newWrap = TextureWrapMode::MirroredRepeat;
        tex->SetWrapMode(newWrap);
        TextureSerializer serializer(tex);
        serializer.Serialize(tab.AssetPath);
    }

    bool mipmaps = tex->GetGenerateMipmaps();
    if (TimeGUI::Checkbox("Generate Mipmaps", &mipmaps))
    {
        tex->SetGenerateMipmaps(mipmaps);
        TextureSerializer serializer(tex);
        serializer.Serialize(tab.AssetPath);
    }

    bool premultAlpha = tex->GetPremultipliedAlpha();
    if (TimeGUI::Checkbox("Premultiplied Alpha", &premultAlpha))
    {
        tex->SetPremultipliedAlpha(premultAlpha);
        TextureSerializer serializer(tex);
        serializer.Serialize(tab.AssetPath);
    }

    TimeGUI::Separator();
    TimeGUI::Text("Source Image Re-linking");

    static char importPathBuffer[512] = "";
    TimeGUI::InputText("Source Path", importPathBuffer, sizeof(importPathBuffer));
    TimeGUI::SameLine();
    if (TimeGUI::Button("Browse..."))
    {
        std::string filepath = PlatformUtils::OpenFile(
            "Image Files (*.png;*.jpg;*.jpeg;*.tga)\0*.png;*.jpg;*.jpeg;*.tga\0All Files "
            "(*.*)\0*.*\0");
        if (!filepath.empty())
        {
            strcpy_s(importPathBuffer, filepath.c_str());

            std::filesystem::path importSrc = filepath;
            if (std::filesystem::exists(importSrc))
            {
                std::filesystem::path destPng = tab.AssetPath;
                destPng.replace_extension(importSrc.extension());

                if (importSrc.extension() != ".png")
                {
                    std::filesystem::path oldPng = tab.AssetPath;
                    oldPng.replace_extension(".png");
                    if (std::filesystem::exists(oldPng))
                        std::filesystem::remove(oldPng);
                }

                std::filesystem::copy_file(importSrc, destPng,
                                           std::filesystem::copy_options::overwrite_existing);

                auto newTex = std::make_shared<Texture>(destPng.string());
                newTex->SetName(tab.AssetPath.stem().string());
                TextureSerializer serializer(newTex);
                serializer.Serialize(tab.AssetPath);

                tab.LoadedAsset = newTex;
                TE_CORE_INFO("Imported texture source from {0}", importSrc.string());
            }
        }
    }

    TimeGUI::EndChild();

    TimeGUI::NextColumn();

    // --- RIGHT PANEL: Canvas Preview, Sliders & Channel Controls ---
    TimeGUI::BeginChild("TextureRightPanel", TEVector2(0.0f, 0.0f), false);

    TimeGUI::Text("Texture Preview Canvas");
    TimeGUI::SameLine();
    TimeGUI::Text(" | Channels:");
    TimeGUI::SameLine();
    TimeGUI::Checkbox("R", &s_ChannelR);
    TimeGUI::SameLine();
    TimeGUI::Checkbox("G", &s_ChannelG);
    TimeGUI::SameLine();
    TimeGUI::Checkbox("B", &s_ChannelB);
    TimeGUI::SameLine();
    TimeGUI::Checkbox("A", &s_ChannelA);

    TimeGUI::SliderFloat("Zoom", &s_CanvasZoom, 0.1f, 5.0f, "%.0f%%");
    TimeGUI::SameLine();
    if (TimeGUI::Button("Reset Zoom"))
    {
        s_CanvasZoom = 1.0f;
    }

    TimeGUI::SliderFloat("UV Tiling", &s_UVTiling, 0.5f, 4.0f, "%.1fx");
    TimeGUI::SameLine();
    if (TimeGUI::Button("Reset Tiling"))
    {
        s_UVTiling = 1.0f;
    }

    TimeGUI::Separator();

    float previewW = (tex->GetWidth() > 0) ? (float)tex->GetWidth() : 256.0f;
    float previewH = (tex->GetHeight() > 0) ? (float)tex->GetHeight() : 256.0f;
    TEVector2 displaySize = TEVector2(previewW * s_CanvasZoom, previewH * s_CanvasZoom);

    TEVector4 tintCol = TEVector4(s_ChannelR ? 1.0f : 0.0f, s_ChannelG ? 1.0f : 0.0f,
                                  s_ChannelB ? 1.0f : 0.0f, s_ChannelA ? 1.0f : 0.0f);

    // Render preview image with dynamic tiling UVs and channel tint mask
    TimeGUI::Image((void *)(uintptr_t)tex->GetRendererID(), displaySize,
                   TEVector2(0.0f, 0.0f), TEVector2(s_UVTiling, s_UVTiling), tintCol);

    TimeGUI::EndChild();
    TimeGUI::Columns(1);
}

TE_REGISTER_ASSET_EDITOR(TextureAssetEditor);

} // namespace TE
