#include "PreRequisites.h"
#include "TextureAssetEditor.hpp"

#include "Log.h"
#include "AssetEditorRegistry.hpp"
#include "Texture.hpp"
#include "TextureSerializer.hpp"
#include "Utils/PlatformUtils.hpp"
#include "Utils/TEFileSystem.hpp"
#include "TimeGUI.hpp"

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
    TimeGUI::Text("Reimport Options");

    if (TimeGUI::Button("Reimport", TEVector2(120.0f, 0.0f)))
    {
        AssetManager::ReimportAsset(tab.AssetPath);
        auto reloadedTex = CreateRef<Texture>();
        TextureSerializer serializer(reloadedTex);
        if (serializer.Deserialize(tab.AssetPath))
        {
            tab.LoadedAsset = reloadedTex;
        }
    }
    TimeGUI::SameLine();
    if (TimeGUI::Button("Reimport With New Source...", TEVector2(180.0f, 0.0f)))
    {
        TEString filepath = PlatformUtils::OpenFile(
            "Image Files (*.png;*.jpg;*.jpeg;*.tga;*.bmp)\0*.png;*.jpg;*.jpeg;*.tga;*.bmp\0All Files (*.*)\0*.*\0");
        if (!filepath.IsEmpty())
        {
            if (AssetManager::ReimportAssetWithNewSource(tab.AssetPath, filepath))
            {
                auto reloadedTex = CreateRef<Texture>();
                TextureSerializer serializer(reloadedTex);
                if (serializer.Deserialize(tab.AssetPath))
                {
                    tab.LoadedAsset = reloadedTex;
                }
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

    float zoomPercent = s_CanvasZoom * 100.0f;
    if (TimeGUI::SliderFloat("Zoom", &zoomPercent, 10.0f, 500.0f, "%.0f%%"))
        s_CanvasZoom = zoomPercent / 100.0f;
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

    TEVector2 canvasPos = TimeGUI::GetCursorScreenPos();
    auto drawList = TimeGUI::GetWindowDrawList();

    // Checkerboard Backdrop
    float tileSize = 16.0f;
    uint32_t colDark = TIMEGUI_COL32(28, 30, 36, 255);
    uint32_t colLight = TIMEGUI_COL32(44, 48, 56, 255);
    drawList.AddRectFilled(canvasPos, TEVector2(canvasPos.x + displaySize.x, canvasPos.y + displaySize.y), colDark);

    for (float y = 0; y < displaySize.y; y += tileSize)
    {
        for (float x = 0; x < displaySize.x; x += tileSize)
        {
            int ix = (int)(x / tileSize);
            int iy = (int)(y / tileSize);
            if ((ix + iy) % 2 == 0)
            {
                float bx1 = canvasPos.x + x;
                float by1 = canvasPos.y + y;
                float bx2 =
                    (bx1 + tileSize > canvasPos.x + displaySize.x) ? canvasPos.x + displaySize.x : bx1 + tileSize;
                float by2 =
                    (by1 + tileSize > canvasPos.y + displaySize.y) ? canvasPos.y + displaySize.y : by1 + tileSize;
                drawList.AddRectFilled(TEVector2(bx1, by1), TEVector2(bx2, by2), colLight);
            }
        }
    }

    // Render preview image with dynamic tiling UVs and channel tint mask
    TimeGUI::Image((void *)(uintptr_t)tex->GetRendererID(), displaySize, TEVector2(0.0f, 0.0f),
                   TEVector2(s_UVTiling, s_UVTiling), tintCol);

    drawList.AddRect(canvasPos, TEVector2(canvasPos.x + displaySize.x, canvasPos.y + displaySize.y),
                     TIMEGUI_COL32(75, 120, 200, 220), 0.0f, 0, 1.5f);

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

bool TextureAssetEditor::CanImportExtension(const TEString &sourceExt) const
{
    return sourceExt.Equals(".png", ESearchCase::IgnoreCase) || sourceExt.Equals(".jpg", ESearchCase::IgnoreCase) ||
           sourceExt.Equals(".jpeg", ESearchCase::IgnoreCase) || sourceExt.Equals(".tga", ESearchCase::IgnoreCase) ||
           sourceExt.Equals(".bmp", ESearchCase::IgnoreCase);
}

TEArray<TEString> TextureAssetEditor::GetSupportedImportExtensions() const
{
    return {".png", ".jpg", ".jpeg", ".tga", ".bmp"};
}

void TextureAssetEditor::OnInitImportConfig(AssetImportConfig &config) const
{
    config.TextureFilterMode = 0; // Linear by default
    config.TextureWrapMode = 0;   // Repeat
    config.GenerateMipmaps = false;
    config.PremultipliedAlpha = false;
}

void TextureAssetEditor::DrawImportSettings(AssetImportConfig &config)
{
    const char *filterItems[] = {"Linear (Smooth / Bilinear)", "Nearest (Pixel Art / Point)"};
    TimeGUI::Combo("Filter Mode", &config.TextureFilterMode, filterItems, 2);

    const char *wrapItems[] = {"Repeat", "Clamp To Edge", "Mirrored Repeat"};
    TimeGUI::Combo("Wrap Mode", &config.TextureWrapMode, wrapItems, 3);

    TimeGUI::Checkbox("Generate Mipmaps", &config.GenerateMipmaps);
    TimeGUI::Checkbox("Premultiplied Alpha", &config.PremultipliedAlpha);
}

TE_REGISTER_ASSET_EDITOR(TextureAssetEditor);
