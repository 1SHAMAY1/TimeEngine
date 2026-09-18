#include "PreRequisites.h"
#include "SpriteSheetAssetEditor.hpp"
#include "AssetEditorRegistry.hpp"
#include "SpriteSheet.hpp"
#include "SpriteSheetSerializer.hpp"
#include "Texture.hpp"
#include "Utils/PlatformUtils.hpp"
#include "Utils/TEFileSystem.hpp"
#include "TimeGUI.hpp"

void SpriteSheetAssetEditor::DrawEditor(EditorTab &tab)
{
    auto sheet = std::dynamic_pointer_cast<SpriteSheet>(tab.LoadedAsset);
    if (!sheet)
    {
        sheet = CreateRef<SpriteSheet>();
        SpriteSheetSerializer serializer(sheet);
        serializer.Deserialize(tab.AssetPath);
        tab.LoadedAsset = sheet;
    }
    if (!sheet)
        return;

    static float s_SheetZoom = 1.0f;
    static int s_ActiveViewMode = 0; // 0 = Slices Overlay, 1 = Animation Player
    static int s_SelectedAnimIdx = 0;
    static bool s_AnimPlaying = false;
    static float s_AnimTimer = 0.0f;
    static uint32_t s_AnimFrameIdx = 0;

    TimeGUI::Columns(2, "SheetResizingColumns", true);

    // --- LEFT PANEL: Controls, Grid Cutter & Anim Sequence Manager ---
    TimeGUI::BeginChild("SheetLeftPanel", TEVector2(0.0f, 0.0f), false);

    TimeGUI::Text("SpriteSheet Settings & Slicer");
    TimeGUI::TextDisabled("Name: %s", sheet->GetName().c_str());
    TimeGUI::SameLine();
    if (TimeGUI::Button("Save SpriteSheet", TEVector2(120.0f, 22.0f)))
    {
        SpriteSheetSerializer serializer(sheet);
        serializer.Serialize(tab.AssetPath);
        AssetEditorRegistry::MarkAssetDirty(tab.AssetPath, false);
    }

    TEString curTexPath = sheet->GetTexturePath();
    if (!curTexPath.empty() && !sheet->GetTexture())
    {
        TEString parentRel = tab.AssetPath.GetParentPath() / curTexPath;
        if (TEFileSystem::Exists(parentRel))
            sheet->SetTexturePath(parentRel);
        else
        {
            TEString fnRel = tab.AssetPath.GetParentPath() / curTexPath.GetFilename();
            if (TEFileSystem::Exists(fnRel))
                sheet->SetTexturePath(fnRel);
        }
    }

    TEString texPathBuf = sheet->GetTexturePath();
    if (TimeGUI::InputText("Source Texture", texPathBuf))
    {
        sheet->SetTexturePath(texPathBuf);
        AssetEditorRegistry::MarkAssetDirty(tab.AssetPath, true);
    }
    TimeGUI::SameLine();
    if (TimeGUI::Button("Browse..."))
    {
        TEString filepath =
            PlatformUtils::OpenFile("Texture Files (*.png;*.jpg;*.tetexture)\0*.png;*.jpg;*.tetexture\0All Files "
                                    "(*.*)\0*.*\0");
        if (!filepath.empty())
        {
            sheet->SetTexturePath(filepath);
            AssetEditorRegistry::MarkAssetDirty(tab.AssetPath, true);
        }
    }

    TimeGUI::Separator();
    TimeGUI::Text("Grid Cutter Parameters");

    int cellW = (int)sheet->GetCellWidth();
    int cellH = (int)sheet->GetCellHeight();
    int padX = (int)sheet->GetPaddingX();
    int padY = (int)sheet->GetPaddingY();
    int offX = (int)sheet->GetOffsetX();
    int offY = (int)sheet->GetOffsetY();

    bool gridChanged = false;
    if (TimeGUI::InputInt("Cell Width", &cellW, 1, 8))
        gridChanged = true;
    if (TimeGUI::InputInt("Cell Height", &cellH, 1, 8))
        gridChanged = true;
    if (TimeGUI::InputInt("Padding X", &padX, 1, 4))
        gridChanged = true;
    if (TimeGUI::InputInt("Padding Y", &padY, 1, 4))
        gridChanged = true;
    if (TimeGUI::InputInt("Offset X", &offX, 1, 4))
        gridChanged = true;
    if (TimeGUI::InputInt("Offset Y", &offY, 1, 4))
        gridChanged = true;

    if (gridChanged)
    {
        sheet->SetGridSettings((uint32_t)cellW, (uint32_t)cellH, (uint32_t)padX, (uint32_t)padY, (uint32_t)offX,
                               (uint32_t)offY);
        sheet->SliceGrid();
        AssetEditorRegistry::MarkAssetDirty(tab.AssetPath, true);
    }

    if (TimeGUI::Button("Slice Grid", 140.0f, 26.0f))
    {
        sheet->SliceGrid();
        AssetEditorRegistry::MarkAssetDirty(tab.AssetPath, true);
    }
    TimeGUI::SameLine();
    if (TimeGUI::Button("Auto Contour Slice", 150.0f, 26.0f))
    {
        sheet->SliceAutoAlpha(0.05f);
        AssetEditorRegistry::MarkAssetDirty(tab.AssetPath, true);
    }

    TimeGUI::Separator();
    TimeGUI::Text("SubFrames (%u frames)", (uint32_t)sheet->GetSubFrames().size());

    // Animation Sequences
    TimeGUI::Separator();
    TimeGUI::Text("Animation Sequences");
    if (TimeGUI::Button("Add Sequence"))
    {
        sheet->AddAnimation("NewAnim");
        s_SelectedAnimIdx = (int)sheet->GetAnimations().size() - 1;
        s_AnimFrameIdx = 0;
        s_AnimTimer = 0.0f;
        AssetEditorRegistry::MarkAssetDirty(tab.AssetPath, true);
    }

    auto &anims = sheet->GetAnimations();
    for (size_t a = 0; a < anims.size(); ++a)
    {
        TimeGUI::PushID((int)a);
        TEString label = anims[a].Name;
        if (TimeGUI::RadioButton(label.c_str(), s_SelectedAnimIdx == (int)a))
        {
            s_SelectedAnimIdx = (int)a;
            s_AnimFrameIdx = 0;
            s_AnimTimer = 0.0f;
        }
        TimeGUI::SameLine();
        if (TimeGUI::Button("X", 22.0f, 22.0f))
        {
            sheet->RemoveAnimation(a);
            AssetEditorRegistry::MarkAssetDirty(tab.AssetPath, true);
            if (s_SelectedAnimIdx >= (int)sheet->GetAnimations().size())
                s_SelectedAnimIdx = (int)sheet->GetAnimations().size() - 1;
            s_AnimFrameIdx = 0;
            s_AnimTimer = 0.0f;
            TimeGUI::PopID();
            break;
        }
        TimeGUI::PopID();
    }

    // --- Sequence Property Inspector ---
    if (!anims.empty() && s_SelectedAnimIdx >= 0 && s_SelectedAnimIdx < (int)anims.size())
    {
        auto &selectedAnim = anims[s_SelectedAnimIdx];
        TimeGUI::Separator();
        TimeGUI::Text("Seq Properties: %s", selectedAnim.Name.c_str());

        if (TimeGUI::InputText("Seq Name", selectedAnim.Name))
        {
            AssetEditorRegistry::MarkAssetDirty(tab.AssetPath, true);
        }

        if (TimeGUI::SliderFloat("Seq FPS", &selectedAnim.FPS, 1.0f, 60.0f, "%.0f FPS"))
        {
            AssetEditorRegistry::MarkAssetDirty(tab.AssetPath, true);
        }

        if (TimeGUI::Checkbox("Loop Sequence", &selectedAnim.Loop))
        {
            AssetEditorRegistry::MarkAssetDirty(tab.AssetPath, true);
        }

        static int startFrame = 0;
        static int endFrame = 0;
        TimeGUI::Text("Assign Frame Range:");
        TimeGUI::InputInt("Start Frame", &startFrame);
        TimeGUI::InputInt("End Frame", &endFrame);

        if (TimeGUI::Button("Apply Frame Range"))
        {
            selectedAnim.FrameIndices.clear();
            int totalFrames = (int)sheet->GetSubFrames().size();
            int sF = (startFrame < 0) ? 0 : startFrame;
            int eF = (endFrame >= totalFrames) ? (totalFrames - 1) : endFrame;
            for (int f = sF; f <= eF; ++f)
            {
                if (f >= 0 && f < totalFrames)
                    selectedAnim.FrameIndices.push_back((uint32_t)f);
            }
            s_AnimFrameIdx = 0;
            s_AnimTimer = 0.0f;
            AssetEditorRegistry::MarkAssetDirty(tab.AssetPath, true);
        }
    }

    TimeGUI::EndChild();

    TimeGUI::NextColumn();

    // --- RIGHT PANEL: Viewport & Interactive Player ---
    TimeGUI::BeginChild("SheetRightPanel", TEVector2(0.0f, 0.0f), false);

    if (TimeGUI::RadioButton("Grid Slices Overlay", s_ActiveViewMode == 0))
        s_ActiveViewMode = 0;
    TimeGUI::SameLine();
    if (TimeGUI::RadioButton("Live Animation Player", s_ActiveViewMode == 1))
        s_ActiveViewMode = 1;

    TimeGUI::SameLine();
    float sheetZoomPercent = s_SheetZoom * 100.0f;
    if (TimeGUI::SliderFloat("Zoom", &sheetZoomPercent, 20.0f, 400.0f, "%.0f%%"))
        s_SheetZoom = sheetZoomPercent / 100.0f;

    TimeGUI::Separator();

    auto tex = sheet->GetTexture();
    if (tex)
    {
        if (s_ActiveViewMode == 0)
        {
            float previewW = (float)tex->GetWidth() * s_SheetZoom;
            float previewH = (float)tex->GetHeight() * s_SheetZoom;
            TEVector2 canvasPos = TimeGUI::GetCursorScreenPos();
            auto drawList = TimeGUI::GetWindowDrawList();

            // Checkerboard Backdrop
            float tileSize = 16.0f;
            uint32_t colDark = TIMEGUI_COL32(28, 30, 36, 255);
            uint32_t colLight = TIMEGUI_COL32(44, 48, 56, 255);
            drawList.AddRectFilled(canvasPos, TEVector2(canvasPos.x + previewW, canvasPos.y + previewH), colDark);

            for (float y = 0; y < previewH; y += tileSize)
            {
                for (float x = 0; x < previewW; x += tileSize)
                {
                    int ix = (int)(x / tileSize);
                    int iy = (int)(y / tileSize);
                    if ((ix + iy) % 2 == 0)
                    {
                        float bx1 = canvasPos.x + x;
                        float by1 = canvasPos.y + y;
                        float bx2 = (bx1 + tileSize > canvasPos.x + previewW) ? canvasPos.x + previewW : bx1 + tileSize;
                        float by2 = (by1 + tileSize > canvasPos.y + previewH) ? canvasPos.y + previewH : by1 + tileSize;
                        drawList.AddRectFilled(TEVector2(bx1, by1), TEVector2(bx2, by2), colLight);
                    }
                }
            }

            TimeGUI::Image((void *)(uintptr_t)tex->GetRendererID(), TEVector2(previewW, previewH), TEVector2(0, 0),
                           TEVector2(1, 1));

            drawList.AddRect(canvasPos, TEVector2(canvasPos.x + previewW, canvasPos.y + previewH),
                             TIMEGUI_COL32(75, 120, 200, 220), 0.0f, 0, 1.5f);

            const auto &subFrames = sheet->GetSubFrames();
            for (const auto &sf : subFrames)
            {
                float minX = canvasPos.x + sf.U0 * previewW;
                float minY = canvasPos.y + sf.V0 * previewH;
                float maxX = canvasPos.x + sf.U1 * previewW;
                float maxY = canvasPos.y + sf.V1 * previewH;

                drawList.AddRect(TEVector2(minX, minY), TEVector2(maxX, maxY), TIMEGUI_COL32(0, 255, 200, 255), 0.0f, 0,
                                 1.5f);
            }
        }
        else
        {
            if (!anims.IsEmpty() && s_SelectedAnimIdx >= 0 && s_SelectedAnimIdx < (int)anims.Num())
            {
                auto &seq = anims[s_SelectedAnimIdx];
                TimeGUI::Text("Playing: %s | FPS: %.0f | Frames: %zu", seq.Name.c_str(), seq.FPS,
                              seq.FrameIndices.Num());
                TimeGUI::SameLine();
                if (TimeGUI::Button(s_AnimPlaying ? "Pause" : "Play"))
                {
                    s_AnimPlaying = !s_AnimPlaying;
                }

                if (s_AnimPlaying && !seq.FrameIndices.IsEmpty())
                {
                    s_AnimTimer += 0.016f;
                    float frameInterval = 1.0f / (seq.FPS > 0.0f ? seq.FPS : 12.0f);
                    if (s_AnimTimer >= frameInterval)
                    {
                        s_AnimTimer = 0.0f;
                        if (seq.Loop)
                        {
                            s_AnimFrameIdx = (s_AnimFrameIdx + 1) % seq.FrameIndices.Num();
                        }
                        else if (s_AnimFrameIdx + 1 < seq.FrameIndices.Num())
                        {
                            s_AnimFrameIdx++;
                        }
                        else
                        {
                            s_AnimPlaying = false;
                        }
                    }
                }

                if (!seq.FrameIndices.IsEmpty())
                {
                    if (s_AnimFrameIdx >= seq.FrameIndices.Num())
                        s_AnimFrameIdx = 0;

                    uint32_t frameIdx = seq.FrameIndices[s_AnimFrameIdx];
                    const auto &subFrames = sheet->GetSubFrames();
                    if (frameIdx < subFrames.Num())
                    {
                        const auto &sf = subFrames[frameIdx];
                        float frameW = (float)sf.Width * s_SheetZoom * 2.0f;
                        float frameH = (float)sf.Height * s_SheetZoom * 2.0f;

                        TimeGUI::Text("Frame #%u: %s", frameIdx, sf.Name.c_str());

                        TimeGUI::Image((void *)(uintptr_t)tex->GetRendererID(), TEVector2(frameW, frameH),
                                       TEVector2(sf.U0, sf.V0), TEVector2(sf.U1, sf.V1));
                    }
                }
            }
        }
    }
    else
    {
        TimeGUI::TextDisabled("No source texture assigned. Assign a texture in the left panel to slice.");
    }

    TimeGUI::EndChild();
    TimeGUI::Columns(1);
}

void SpriteSheetAssetEditor::DrawIcon(const TEVector2 &min, const TEVector2 &max) const
{
    TimeGUI::TimeGUIDrawList dl = TimeGUI::GetWindowDrawList();
    float w = max.x - min.x;
    float h = max.y - min.y;
    float pad = w * 0.12f;

    // Background Card
    dl.AddRectFilled(min, max, IM_COL32(245, 160, 40, 230), 4.0f);
    // 2x2 Grid of Sprite Cells
    float cellW = (w - pad * 3.0f) * 0.5f;
    float cellH = (h - pad * 3.0f) * 0.5f;
    for (int y = 0; y < 2; ++y)
    {
        for (int x = 0; x < 2; ++x)
        {
            TEVector2 cMin(min.x + pad + x * (cellW + pad), min.y + pad + y * (cellH + pad));
            TEVector2 cMax(cMin.x + cellW, cMin.y + cellH);
            dl.AddRectFilled(cMin, cMax, IM_COL32(35, 35, 42, 255), 1.5f);
            dl.AddRect(cMin, cMax, IM_COL32(255, 215, 100, 200), 1.5f, 0, 1.0f);
        }
    }
}

TE_REGISTER_ASSET_EDITOR(SpriteSheetAssetEditor);
