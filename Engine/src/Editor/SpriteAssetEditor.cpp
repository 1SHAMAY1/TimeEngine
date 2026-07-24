#include "Editor/SpriteAssetEditor.hpp"
#include "Editor/AssetEditorRegistry.hpp"
#include "Renderer/Sprite.hpp"
#include "Renderer/SpriteSerializer.hpp"
#include "Renderer/Texture.hpp"
#include "Utils/PlatformUtils.hpp"
#include "Utils/TimeGUI.hpp"
#include <imgui.h>
#include <filesystem>
#include <string>

namespace TE
{

void SpriteAssetEditor::DrawEditor(EditorTab &tab)
{
    auto sprite = std::dynamic_pointer_cast<Sprite>(tab.LoadedAsset);
    if (!sprite) return;

    static float s_SpriteZoom = 1.0f;
    static int s_SpriteViewMode = 0; // 0 = Preview, 1 = Pivot Tool, 2 = Collider Editor

    TimeGUI::Columns(2, "SpriteResizingColumns", true);

    // --- LEFT PANEL: Sprite Parameters & Collider Editor ---
    TimeGUI::BeginChild("SpriteLeftPanel", TEVector2(0.0f, 0.0f), false);

    TimeGUI::Text("Sprite Asset Inspector");
    TimeGUI::TextDisabled("Name: %s", sprite->GetName().c_str());

    std::string curTexPath = sprite->GetTexturePath();
    static char spriteTexBuf[512] = "";
    strcpy_s(spriteTexBuf, curTexPath.c_str());
    if (TimeGUI::InputText("Source Texture", spriteTexBuf, sizeof(spriteTexBuf)))
    {
        sprite->SetTexturePath(spriteTexBuf);
        SpriteSerializer serializer(sprite);
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
            strcpy_s(spriteTexBuf, filepath.c_str());
            sprite->SetTexturePath(filepath);
            SpriteSerializer serializer(sprite);
            serializer.Serialize(tab.AssetPath);
        }
    }

    TimeGUI::Separator();
    TimeGUI::Text("UV Bounds");

    float u0, v0, u1, v1;
    sprite->GetUVs(u0, v0, u1, v1);
    bool uvChanged = false;
    if (TimeGUI::SliderFloat("U0 (Left)", &u0, 0.0f, 1.0f, "%.3f")) uvChanged = true;
    if (TimeGUI::SliderFloat("V0 (Top)", &v0, 0.0f, 1.0f, "%.3f")) uvChanged = true;
    if (TimeGUI::SliderFloat("U1 (Right)", &u1, 0.0f, 1.0f, "%.3f")) uvChanged = true;
    if (TimeGUI::SliderFloat("V1 (Bottom)", &v1, 0.0f, 1.0f, "%.3f")) uvChanged = true;

    if (uvChanged)
    {
        sprite->SetUVs(u0, v0, u1, v1);
        SpriteSerializer serializer(sprite);
        serializer.Serialize(tab.AssetPath);
    }

    if (TimeGUI::Button("Reset Full Image UVs (0..1)"))
    {
        sprite->SetUVs(0.0f, 0.0f, 1.0f, 1.0f);
        SpriteSerializer serializer(sprite);
        serializer.Serialize(tab.AssetPath);
    }

    TimeGUI::Separator();
    TimeGUI::Text("Pivot Anchor Point");

    float px, py;
    sprite->GetPivot(px, py);
    bool pivotChanged = false;
    if (TimeGUI::SliderFloat("Pivot X", &px, 0.0f, 1.0f, "%.2f")) pivotChanged = true;
    if (TimeGUI::SliderFloat("Pivot Y", &py, 0.0f, 1.0f, "%.2f")) pivotChanged = true;

    if (pivotChanged)
    {
        sprite->SetPivot(px, py);
        SpriteSerializer serializer(sprite);
        serializer.Serialize(tab.AssetPath);
    }

    TimeGUI::Text("Pivot Presets:");
    if (TimeGUI::Button("Center (0.5, 0.5)"))
    {
        sprite->SetPivot(0.5f, 0.5f);
        SpriteSerializer serializer(sprite);
        serializer.Serialize(tab.AssetPath);
    }
    TimeGUI::SameLine();
    if (TimeGUI::Button("Bottom-Center (0.5, 0.0)"))
    {
        sprite->SetPivot(0.5f, 0.0f);
        SpriteSerializer serializer(sprite);
        serializer.Serialize(tab.AssetPath);
    }
    if (TimeGUI::Button("Top-Left (0.0, 1.0)"))
    {
        sprite->SetPivot(0.0f, 1.0f);
        SpriteSerializer serializer(sprite);
        serializer.Serialize(tab.AssetPath);
    }

    TimeGUI::Separator();
    TimeGUI::Text("Pixels Per Unit (PPU)");
    float ppu = sprite->GetPixelsPerUnit();
    if (TimeGUI::SliderFloat("PPU", &ppu, 1.0f, 256.0f, "%.0f PPU"))
    {
        sprite->SetPixelsPerUnit(ppu);
        SpriteSerializer serializer(sprite);
        serializer.Serialize(tab.AssetPath);
    }

    TimeGUI::Separator();
    TimeGUI::Text("2D Physics Polygon Collider");
    if (TimeGUI::Button("Auto Box Contour"))
    {
        sprite->GenerateAutoContourCollider(0.1f);
        SpriteSerializer serializer(sprite);
        serializer.Serialize(tab.AssetPath);
    }
    TimeGUI::SameLine();
    if (TimeGUI::Button("Clear Points"))
    {
        sprite->GetCustomColliderPoints().clear();
        SpriteSerializer serializer(sprite);
        serializer.Serialize(tab.AssetPath);
    }

    if (TimeGUI::Button("Add Vertex Point"))
    {
        sprite->GetCustomColliderPoints().push_back({ 0.0f, 0.0f });
        SpriteSerializer serializer(sprite);
        serializer.Serialize(tab.AssetPath);
    }

    auto &pts = sprite->GetCustomColliderPoints();
    for (size_t ptIdx = 0; ptIdx < pts.size(); ++ptIdx)
    {
        TimeGUI::PushID((int)ptIdx);
        bool ptChanged = false;
        std::string labelX = "Pt #" + std::to_string(ptIdx) + " X";
        std::string labelY = "Pt #" + std::to_string(ptIdx) + " Y";
        if (TimeGUI::SliderFloat(labelX.c_str(), &pts[ptIdx].x, -1.0f, 1.0f, "%.2f")) ptChanged = true;
        if (TimeGUI::SliderFloat(labelY.c_str(), &pts[ptIdx].y, -1.0f, 1.0f, "%.2f")) ptChanged = true;
        if (ptChanged)
        {
            SpriteSerializer serializer(sprite);
            serializer.Serialize(tab.AssetPath);
        }
        TimeGUI::PopID();
    }

    TimeGUI::EndChild();

    TimeGUI::NextColumn();

    // --- RIGHT PANEL: Interactive Viewport Canvas ---
    TimeGUI::BeginChild("SpriteRightPanel", TEVector2(0.0f, 0.0f), false);

    if (TimeGUI::RadioButton("Preview", s_SpriteViewMode == 0)) s_SpriteViewMode = 0;
    TimeGUI::SameLine();
    if (TimeGUI::RadioButton("Pivot Tool", s_SpriteViewMode == 1)) s_SpriteViewMode = 1;
    TimeGUI::SameLine();
    if (TimeGUI::RadioButton("Collider Editor", s_SpriteViewMode == 2)) s_SpriteViewMode = 2;

    TimeGUI::SameLine();
    TimeGUI::SliderFloat("Zoom", &s_SpriteZoom, 0.2f, 5.0f, "%.0f%%");

    TimeGUI::Separator();

    auto tex = sprite->GetTexture();
    if (tex)
    {
        float fullW = (float)tex->GetWidth() * s_SpriteZoom;
        float fullH = (float)tex->GetHeight() * s_SpriteZoom;

        float su0, sv0, su1, sv1;
        sprite->GetUVs(su0, sv0, su1, sv1);

        TEVector2 canvasPos = TimeGUI::GetCursorScreenPos();

        // Draw cropped sprite image
        TimeGUI::Image((void *)(uintptr_t)tex->GetRendererID(), TEVector2(fullW, fullH),
                       TEVector2(su0, sv0), TEVector2(su1, sv1));

        bool isCanvasHovered = TimeGUI::IsItemHovered();
        bool isMouseDown = TimeGUI::IsMouseDown(0);
        TEVector2 mousePos = TimeGUI::GetMousePos();

        auto drawList = TimeGUI::GetWindowDrawList();

        // Mode 1: Pivot Tool (Draw yellow crosshair + Direct Canvas Dragging)
        if (s_SpriteViewMode == 1)
        {
            float spx, spy;
            sprite->GetPivot(spx, spy);

            // Direct Mouse Dragging for Pivot
            if (isCanvasHovered && isMouseDown)
            {
                float newPx = (mousePos.x - canvasPos.x) / fullW;
                float newPy = 1.0f - ((mousePos.y - canvasPos.y) / fullH);
                newPx = (newPx < 0.0f) ? 0.0f : ((newPx > 1.0f) ? 1.0f : newPx);
                newPy = (newPy < 0.0f) ? 0.0f : ((newPy > 1.0f) ? 1.0f : newPy);

                sprite->SetPivot(newPx, newPy);
                SpriteSerializer serializer(sprite);
                serializer.Serialize(tab.AssetPath);
                spx = newPx;
                spy = newPy;
            }

            float pivotScreenX = canvasPos.x + spx * fullW;
            float pivotScreenY = canvasPos.y + (1.0f - spy) * fullH; // top-down Y

            drawList->AddLine(ImVec2(pivotScreenX - 15.0f, pivotScreenY), ImVec2(pivotScreenX + 15.0f, pivotScreenY), IM_COL32(255, 230, 0, 255), 2.5f);
            drawList->AddLine(ImVec2(pivotScreenX, pivotScreenY - 15.0f), ImVec2(pivotScreenX, pivotScreenY + 15.0f), IM_COL32(255, 230, 0, 255), 2.5f);
            drawList->AddCircleFilled(ImVec2(pivotScreenX, pivotScreenY), 6.0f, IM_COL32(255, 230, 0, 255));
            drawList->AddCircle(ImVec2(pivotScreenX, pivotScreenY), 12.0f, IM_COL32(255, 230, 0, 255), 0, 1.5f);
        }
        // Mode 2: Physics Polygon Collider Editor (Draw red wireframe + Direct Dragging Vertices)
        else if (s_SpriteViewMode == 2)
        {
            auto &cPoints = sprite->GetCustomColliderPoints();
            static int s_DraggedVertexIdx = -1;

            if (!isMouseDown)
            {
                s_DraggedVertexIdx = -1;
            }

            // Check dragging or selecting vertex
            if (isCanvasHovered && isMouseDown && s_DraggedVertexIdx == -1)
            {
                for (size_t i = 0; i < cPoints.size(); ++i)
                {
                    float vx = canvasPos.x + (cPoints[i].x + 0.5f) * fullW;
                    float vy = canvasPos.y + (0.5f - cPoints[i].y) * fullH;
                    float distSq = (mousePos.x - vx) * (mousePos.x - vx) + (mousePos.y - vy) * (mousePos.y - vy);
                    if (distSq <= 144.0f) // 12px radius handle
                    {
                        s_DraggedVertexIdx = (int)i;
                        break;
                    }
                }
            }

            // Perform Dragging
            if (s_DraggedVertexIdx >= 0 && s_DraggedVertexIdx < (int)cPoints.size() && isMouseDown)
            {
                float newX = (mousePos.x - canvasPos.x) / fullW - 0.5f;
                float newY = 0.5f - ((mousePos.y - canvasPos.y) / fullH);
                cPoints[s_DraggedVertexIdx] = { newX, newY };
                SpriteSerializer serializer(sprite);
                serializer.Serialize(tab.AssetPath);
            }

            // Render wireframe and handles
            if (cPoints.size() >= 2)
            {
                for (size_t i = 0; i < cPoints.size(); ++i)
                {
                    size_t nextIdx = (i + 1) % cPoints.size();
                    TEVector2 p1 = cPoints[i];
                    TEVector2 p2 = cPoints[nextIdx];

                    float x1 = canvasPos.x + (p1.x + 0.5f) * fullW;
                    float y1 = canvasPos.y + (0.5f - p1.y) * fullH;
                    float x2 = canvasPos.x + (p2.x + 0.5f) * fullW;
                    float y2 = canvasPos.y + (0.5f - p2.y) * fullH;

                    drawList->AddLine(ImVec2(x1, y1), ImVec2(x2, y2), IM_COL32(255, 60, 60, 255), 2.5f);

                    uint32_t handleColor = (s_DraggedVertexIdx == (int)i) ? IM_COL32(255, 255, 0, 255) : IM_COL32(255, 60, 60, 255);
                    drawList->AddCircleFilled(ImVec2(x1, y1), 6.0f, handleColor);
                    drawList->AddCircle(ImVec2(x1, y1), 10.0f, handleColor, 0, 1.5f);
                }
            }
        }
    }
    else
    {
        TimeGUI::TextDisabled("No source texture assigned. Assign a texture in the left panel.");
    }

    TimeGUI::EndChild();
    TimeGUI::Columns(1);
}

TE_REGISTER_ASSET_EDITOR(SpriteAssetEditor);

} // namespace TE
