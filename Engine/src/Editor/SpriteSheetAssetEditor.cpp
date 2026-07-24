#include "Editor/SpriteSheetAssetEditor.hpp"
#include "Editor/AssetEditorRegistry.hpp"
#include "Renderer/SpriteSheet.hpp"
#include "Renderer/SpriteSheetSerializer.hpp"
#include "Renderer/Texture.hpp"
#include "Utils/PlatformUtils.hpp"
#include "Utils/TimeGUI.hpp"
#include <imgui.h>
#include <filesystem>
#include <string>

namespace TE
{

void SpriteSheetAssetEditor::DrawEditor(EditorTab &tab)
{
    auto sheet = std::dynamic_pointer_cast<SpriteSheet>(tab.LoadedAsset);
    if (!sheet) return;

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

    std::string curTexPath = sheet->GetTexturePath();
    static char texPathBuf[512] = "";
    strcpy_s(texPathBuf, curTexPath.c_str());
    if (TimeGUI::InputText("Source Texture", texPathBuf, sizeof(texPathBuf)))
    {
        sheet->SetTexturePath(texPathBuf);
        SpriteSheetSerializer serializer(sheet);
        serializer.Serialize(tab.AssetPath);
    }
    TimeGUI::SameLine();
    if (TimeGUI::Button("Browse..."))
    {
        std::string filepath = PlatformUtils::OpenFile(
            "Texture Files (*.png;*.jpg;*.tetexture)\0*.png;*.jpg;*.tetexture\0All Files "
            "(*.*)\0*.*\0");
        if (!filepath.empty())
        {
            strcpy_s(texPathBuf, filepath.c_str());
            sheet->SetTexturePath(filepath);
            SpriteSheetSerializer serializer(sheet);
            serializer.Serialize(tab.AssetPath);
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
        sheet->SetGridSettings((uint32_t)cellW, (uint32_t)cellH, (uint32_t)padX, (uint32_t)padY,
                               (uint32_t)offX, (uint32_t)offY);
        sheet->SliceGrid();
        SpriteSheetSerializer serializer(sheet);
        serializer.Serialize(tab.AssetPath);
    }

    if (TimeGUI::Button("Slice Grid", 140.0f, 26.0f))
    {
        sheet->SliceGrid();
        SpriteSheetSerializer serializer(sheet);
        serializer.Serialize(tab.AssetPath);
    }
    TimeGUI::SameLine();
    if (TimeGUI::Button("Auto Contour Slice", 150.0f, 26.0f))
    {
        sheet->SliceAutoAlpha(0.05f);
        SpriteSheetSerializer serializer(sheet);
        serializer.Serialize(tab.AssetPath);
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
        SpriteSheetSerializer serializer(sheet);
        serializer.Serialize(tab.AssetPath);
    }

    auto &anims = sheet->GetAnimations();
    for (size_t a = 0; a < anims.size(); ++a)
    {
        TimeGUI::PushID((int)a);
        std::string label = anims[a].Name;
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
            SpriteSheetSerializer serializer(sheet);
            serializer.Serialize(tab.AssetPath);
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

        static char animNameBuf[128] = "";
        strcpy_s(animNameBuf, selectedAnim.Name.c_str());
        if (TimeGUI::InputText("Seq Name", animNameBuf, sizeof(animNameBuf)))
        {
            selectedAnim.Name = animNameBuf;
            SpriteSheetSerializer serializer(sheet);
            serializer.Serialize(tab.AssetPath);
        }

        if (TimeGUI::SliderFloat("Seq FPS", &selectedAnim.FPS, 1.0f, 60.0f, "%.0f FPS"))
        {
            SpriteSheetSerializer serializer(sheet);
            serializer.Serialize(tab.AssetPath);
        }

        if (TimeGUI::Checkbox("Loop Sequence", &selectedAnim.Loop))
        {
            SpriteSheetSerializer serializer(sheet);
            serializer.Serialize(tab.AssetPath);
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
            SpriteSheetSerializer serializer(sheet);
            serializer.Serialize(tab.AssetPath);
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
    TimeGUI::SliderFloat("Zoom", &s_SheetZoom, 0.2f, 4.0f, "%.0f%%");

    TimeGUI::Separator();

    auto tex = sheet->GetTexture();
    if (tex)
    {
        if (s_ActiveViewMode == 0)
        {
            float previewW = (float)tex->GetWidth() * s_SheetZoom;
            float previewH = (float)tex->GetHeight() * s_SheetZoom;
            TEVector2 canvasPos = TimeGUI::GetCursorScreenPos();
            TimeGUI::Image((void *)(uintptr_t)tex->GetRendererID(), TEVector2(previewW, previewH),
                           TEVector2(0, 0), TEVector2(1, 1));

            auto drawList = TimeGUI::GetWindowDrawList();
            const auto &subFrames = sheet->GetSubFrames();
            for (const auto &sf : subFrames)
            {
                float minX = canvasPos.x + sf.U0 * previewW;
                float minY = canvasPos.y + sf.V0 * previewH;
                float maxX = canvasPos.x + sf.U1 * previewW;
                float maxY = canvasPos.y + sf.V1 * previewH;

                drawList->AddRect(ImVec2(minX, minY), ImVec2(maxX, maxY), IM_COL32(0, 255, 200, 255), 0.0f, 0, 1.5f);
            }
        }
        else
        {
            if (!anims.empty() && s_SelectedAnimIdx >= 0 && s_SelectedAnimIdx < (int)anims.size())
            {
                auto &seq = anims[s_SelectedAnimIdx];
                TimeGUI::Text("Playing: %s | FPS: %.0f | Frames: %zu", seq.Name.c_str(), seq.FPS, seq.FrameIndices.size());
                TimeGUI::SameLine();
                if (TimeGUI::Button(s_AnimPlaying ? "Pause" : "Play"))
                {
                    s_AnimPlaying = !s_AnimPlaying;
                }

                if (s_AnimPlaying && !seq.FrameIndices.empty())
                {
                    s_AnimTimer += 0.016f;
                    float frameInterval = 1.0f / (seq.FPS > 0.0f ? seq.FPS : 12.0f);
                    if (s_AnimTimer >= frameInterval)
                    {
                        s_AnimTimer = 0.0f;
                        if (seq.Loop)
                        {
                            s_AnimFrameIdx = (s_AnimFrameIdx + 1) % seq.FrameIndices.size();
                        }
                        else if (s_AnimFrameIdx + 1 < seq.FrameIndices.size())
                        {
                            s_AnimFrameIdx++;
                        }
                        else
                        {
                            s_AnimPlaying = false;
                        }
                    }
                }

                if (!seq.FrameIndices.empty())
                {
                    if (s_AnimFrameIdx >= seq.FrameIndices.size())
                        s_AnimFrameIdx = 0;

                    uint32_t frameIdx = seq.FrameIndices[s_AnimFrameIdx];
                    const auto &subFrames = sheet->GetSubFrames();
                    if (frameIdx < subFrames.size())
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

TE_REGISTER_ASSET_EDITOR(SpriteSheetAssetEditor);

} // namespace TE
