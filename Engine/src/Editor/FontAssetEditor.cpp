#include "Editor/FontAssetEditor.hpp"
#include "Core/Asset/FontAsset.hpp"
#include "Core/PreRequisites.h"
#include "Editor/AssetEditorRegistry.hpp"
#include "Utils/PlatformUtils.hpp"
#include "Utils/TimeGUI.hpp"

void FontAssetEditor::DrawEditor(EditorTab &tab)
{
    auto fontAsset = std::dynamic_pointer_cast<FontAsset>(tab.LoadedAsset);
    if (!fontAsset)
        return;

    TimeGUI::TextColored(TEColor(0.2f, 0.8f, 1.0f, 1.0f), "FontAsset: %s", fontAsset->GetName().c_str());
    TimeGUI::SameLine();
    TimeGUI::TextDisabled("(Pixel Size: %.1f | Line Height: %.1f | Ascent: %.1f | Descent: %.1f)",
                          fontAsset->GetPixelSize(), fontAsset->GetLineHeight(), fontAsset->GetAscent(),
                          fontAsset->GetDescent());

    TimeGUI::Separator();

    static TEString s_SampleText = "The quick brown fox jumps over the lazy dog! 1234567890";
    static float s_TestScale = 1.0f;
    static float s_RebakePixelSize = 32.0f;

    // Controls
    TimeGUI::Text("Source TTF: %s", fontAsset->GetSourcePath().empty() ? "(None)" : fontAsset->GetSourcePath().c_str());
    TimeGUI::SameLine();
    if (TimeGUI::Button("Select TTF File..."))
    {
        TEString ttfPath = PlatformUtils::OpenFile("Font Files (*.ttf;*.otf)\0*.ttf;*.otf\0");
        if (!ttfPath.empty())
        {
            fontAsset->BakeFromTTF(ttfPath, fontAsset->GetPixelSize());
            AssetEditorRegistry::MarkAssetDirty(tab.AssetPath, true);
        }
    }

    TimeGUI::SameLine();
    TimeGUI::SetNextItemWidth(100.0f);
    TimeGUI::DragFloat("Bake Size##PixelSize", &s_RebakePixelSize, 1.0f, 12.0f, 128.0f, "%.0f px");
    TimeGUI::SameLine();
    if (TimeGUI::Button("Rebake Atlas"))
    {
        if (!fontAsset->GetSourcePath().empty())
        {
            fontAsset->BakeFromTTF(fontAsset->GetSourcePath(), s_RebakePixelSize);
            AssetEditorRegistry::MarkAssetDirty(tab.AssetPath, true);
        }
    }

    TimeGUI::SameLine();
    if (TimeGUI::Button("Save Font"))
    {
        if (fontAsset->SaveToFile(tab.AssetPath))
        {
            AssetEditorRegistry::MarkAssetDirty(tab.AssetPath, false);
        }
    }

    TimeGUI::Separator();

    // Two Column layout: Left = Live Typography Tester, Right = Texture Atlas Preview
    TimeGUI::Columns(2, "FontEditorColumns", true);

    // Left Column
    TimeGUI::Text("Live Typography Preview:");
    TimeGUI::SetNextItemWidth(-1.0f);
    TimeGUI::InputText("##SampleText", s_SampleText);
    TimeGUI::SliderFloat("Preview Scale", &s_TestScale, 0.5f, 3.0f, "%.2fx");

    TEVector2 textSize = fontAsset->MeasureString(s_SampleText, s_TestScale);
    TimeGUI::TextDisabled("Measured Bounds: %.1f x %.1f px", textSize.x, textSize.y);

    // Render Preview Box
    TEVector2 canvasP0 = TimeGUI::GetCursorScreenPos();
    TEVector2 canvasSz = TEVector2(TimeGUI::GetContentRegionAvail().x, 200.0f);
    TEVector2 canvasP1 = TEVector2(canvasP0.x + canvasSz.x, canvasP0.y + canvasSz.y);

    TimeGUIDrawList drawList = TimeGUI::GetWindowDrawList();
    drawList.AddRectFilled(canvasP0, canvasP1, TIMEGUI_COL32(20, 24, 30, 255));
    drawList.AddRect(canvasP0, canvasP1, TIMEGUI_COL32(60, 70, 85, 255));

    auto atlasTex = fontAsset->GetAtlasTexture();
    if (atlasTex && atlasTex->GetRendererID() != 0)
    {
        TimeGUITextureID texId = (TimeGUITextureID)(intptr_t)atlasTex->GetRendererID();
        float cursorX = canvasP0.x + 10.0f;
        float cursorY = canvasP0.y + 30.0f + fontAsset->GetAscent() * s_TestScale;
        TEString prevChar = "";

        TEString sampleText(s_SampleText);
        for (size_t i = 0; i < sampleText.Len(); ++i)
        {
            TEString curChar(&sampleText.c_str()[i], 1);
            if (!prevChar.IsEmpty())
            {
                cursorX += fontAsset->GetKerning(prevChar, curChar) * s_TestScale;
            }

            FontGlyph glyph;
            if (fontAsset->GetGlyph(curChar, glyph))
            {
                float x0 = cursorX + glyph.BearingX * s_TestScale;
                float y0 = cursorY - glyph.BearingY * s_TestScale;
                float x1 = x0 + glyph.Width * s_TestScale;
                float y1 = y0 + glyph.Height * s_TestScale;

                drawList.AddImage(texId, TEVector2(x0, y0), TEVector2(x1, y1), TEVector2(glyph.UV.x, glyph.UV.y),
                                  TEVector2(glyph.UV.z, glyph.UV.w), TIMEGUI_COL32(255, 255, 255, 255));

                cursorX += glyph.AdvanceX * s_TestScale;
            }
            prevChar = curChar;
        }
    }
    TimeGUI::Dummy(canvasSz);

    TimeGUI::NextColumn();

    // Right Column: Texture Atlas View
    TimeGUI::Text("Glyph Atlas Texture (%u x %u):", fontAsset->GetAtlasWidth(), fontAsset->GetAtlasHeight());
    if (atlasTex && atlasTex->GetRendererID() != 0)
    {
        TimeGUITextureID texId = (TimeGUITextureID)(intptr_t)atlasTex->GetRendererID();
        float availW = TimeGUI::GetContentRegionAvail().x;
        float previewH = availW * ((float)fontAsset->GetAtlasHeight() / (float)fontAsset->GetAtlasWidth());
        TimeGUI::Image(texId, TEVector2(availW, previewH), TEVector2(0, 0), TEVector2(1, 1));
    }
    else
    {
        TimeGUI::TextDisabled("(No Atlas Generated - select a TTF file to bake)");
    }

    TimeGUI::Columns(1);
}

void FontAssetEditor::DrawIcon(const TEVector2 &min, const TEVector2 &max) const
{
    TimeGUI::TimeGUIDrawList dl = TimeGUI::GetWindowDrawList();
    float w = max.x - min.x;
    float h = max.y - min.y;
    float pad = w * 0.12f;

    // Card background in Crimson / Rose
    dl.AddRectFilled(min, max, IM_COL32(210, 50, 80, 230), 4.0f);
    TEVector2 iMin(min.x + pad, min.y + pad);
    TEVector2 iMax(max.x - pad, max.y - pad);
    dl.AddRectFilled(iMin, iMax, IM_COL32(35, 20, 28, 255), 2.0f);

    // Bold Typography 'A'
    float cx = (iMin.x + iMax.x) * 0.5f;
    float cy = (iMin.y + iMax.y) * 0.5f;
    dl.AddTriangleFilled(TEVector2(cx, iMin.y + 4.0f), TEVector2(iMin.x + 4.0f, iMax.y - 4.0f),
                         TEVector2(iMax.x - 4.0f, iMax.y - 4.0f), IM_COL32(255, 120, 150, 255));
    dl.AddTriangleFilled(TEVector2(cx, iMin.y + 10.0f), TEVector2(cx - 3.0f, cy + 3.0f),
                         TEVector2(cx + 3.0f, cy + 3.0f), IM_COL32(35, 20, 28, 255));
}

TE_REGISTER_ASSET_EDITOR(FontAssetEditor);
