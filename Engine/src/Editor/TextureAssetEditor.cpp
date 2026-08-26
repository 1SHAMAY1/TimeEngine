#include "Core/PreRequisites.h"
#include "Editor/TextureAssetEditor.hpp"

#include "Core/Log.h"
#include "Editor/AssetEditorRegistry.hpp"
#include "Renderer/Texture.hpp"
#include "Renderer/TextureSerializer.hpp"
#include "Utils/PlatformUtils.hpp"
#include "Utils/TEFileSystem.hpp"
#include "Utils/TimeGUI.hpp"

void TextureAssetEditor::DrawEditor(EditorTab &tab)
{
    auto tex = std::dynamic_pointer_cast<Texture>(tab.LoadedAsset);
    if (!tex)
    {
        tex = CreateRef<Texture>(tab.AssetPath);
        tab.LoadedAsset = tex;
    }
    if (!tex)
    {
        TimeGUI::TextDisabled("Failed to load texture: %s", tab.AssetPath.c_str());
        return;
    }

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

    static TEString importPathBuffer;
    TimeGUI::InputText("Source Path", importPathBuffer);
    TimeGUI::SameLine();
    if (TimeGUI::Button("Browse..."))
    {
        TEString filepath =
            PlatformUtils::OpenFile("Image Files (*.png;*.jpg;*.jpeg;*.tga)\0*.png;*.jpg;*.jpeg;*.tga\0All Files "
                                    "(*.*)\0*.*\0");
        if (!filepath.IsEmpty())
        {
            importPathBuffer = filepath;

            TEString importSrc = filepath;
            if (TEFileSystem::Exists(importSrc))
            {
                TEString destPng = tab.AssetPath.ReplaceExtension(importSrc.GetExtension());

                if (importSrc.GetExtension() != ".png")
                {
                    TEString oldPng = tab.AssetPath.ReplaceExtension(".png");
                    if (TEFileSystem::Exists(oldPng))
                        TEFileSystem::Remove(oldPng);
                }

                TEFileSystem::CopyFile(importSrc, destPng, true);

                auto newTex = CreateRef<Texture>(destPng);
                newTex->SetName(tab.AssetPath.GetStem());
                TextureSerializer serializer(newTex);
                serializer.Serialize(tab.AssetPath);

                tab.LoadedAsset = newTex;
                TE_CORE_INFO("Imported texture source from {0}", importSrc.c_str());
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

    TEVector4 tintCol = TEVector4(s_ChannelR ? 1.0f : 0.0f, s_ChannelG ? 1.0f : 0.0f, s_ChannelB ? 1.0f : 0.0f,
                                  s_ChannelA ? 1.0f : 0.0f);

    // Render preview image with dynamic tiling UVs and channel tint mask
    TimeGUI::Image((void *)(uintptr_t)tex->GetRendererID(), displaySize, TEVector2(0.0f, 0.0f),
                   TEVector2(s_UVTiling, s_UVTiling), tintCol);

    TimeGUI::EndChild();
    TimeGUI::Columns(1);
}

void TextureAssetEditor::DrawIcon(const TEVector2 &min, const TEVector2 &max) const
{
    TimeGUI::TimeGUIDrawList dl = TimeGUI::GetWindowDrawList();
    float w = max.x - min.x;
    float h = max.y - min.y;
    float pad = w * 0.12f;

    // Outer Photo Frame in Slate Blue
    dl.AddRectFilled(min, max, IM_COL32(50, 140, 230, 230), 4.0f);
    TEVector2 iMin(min.x + pad, min.y + pad);
    TEVector2 iMax(max.x - pad, max.y - pad);
    dl.AddRectFilled(iMin, iMax, IM_COL32(25, 30, 45, 255), 2.0f);

    // Sun
    dl.AddCircleFilled(TEVector2(iMin.x + (iMax.x - iMin.x) * 0.72f, iMin.y + (iMax.y - iMin.y) * 0.32f),
                       (iMax.x - iMin.x) * 0.14f, IM_COL32(255, 220, 80, 255));
    // Mountain Peak
    dl.AddTriangleFilled(TEVector2(iMin.x + (iMax.x - iMin.x) * 0.15f, iMax.y),
                         TEVector2(iMin.x + (iMax.x - iMin.x) * 0.50f, iMin.y + (iMax.y - iMin.y) * 0.40f),
                         TEVector2(iMin.x + (iMax.x - iMin.x) * 0.85f, iMax.y), IM_COL32(60, 180, 140, 255));
}

TE_REGISTER_ASSET_EDITOR(TextureAssetEditor);
