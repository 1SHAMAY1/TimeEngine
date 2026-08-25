#include "VectorEditorSubmode.hpp"
#include "../SpriteEditorLayer.hpp"
#include "../SpriteMode.hpp"
#include "Input/Input.hpp"
#include "Utils/TimeGUI.hpp"
#include <algorithm>


void VectorEditorSubmode::OnEnter(SpriteMode *mode)
{
}

void VectorEditorSubmode::OnUpdate(float dt, SpriteMode *mode)
{
}

void VectorEditorSubmode::OnExit(SpriteMode *mode)
{
}

bool VectorEditorSubmode::OnShortcut(const TEString &shortcutId, SpriteMode *mode)
{
    return false;
}

void VectorEditorSubmode::OnTimeGUIRender(SpriteEditorLayer *layer, SpriteMode *mode)
{
    if (!mode)
        return;

    if (TimeGUI::BeginTable("##VectorLayout", 3, TimeGUITableFlags_Resizable))
    {
        // 1. Tool Sidebar
        TimeGUI::TableNextColumn();
        DrawToolSidebar(mode);

        // 2. Vector Canvas
        TimeGUI::TableNextColumn();
        DrawVectorCanvas(mode);

        // 3. Properties Sidebar
        TimeGUI::TableNextColumn();
        DrawPropertiesSidebar(mode);

        TimeGUI::EndTable();
    }
}

void VectorEditorSubmode::DrawToolSidebar(SpriteMode *mode)
{
    TimeGUI::BeginChild("##VecTools", TEVector2(0, 0), true);
    TimeGUI::TextColored(TEVector4(0.4f, 0.8f, 1.0f, 1.0f), "VECTOR TOOLS");
    TimeGUI::Separator();

    auto ToolBtn = [&](const TEString& label, VectorShapeType type)
    {
        bool active = (mode->m_ActiveVectorTool == type);
        if (active)
        {
            TimeGUI::PushStyleColor(TimeGUICol_Button, TEVector4(0.2f, 0.5f, 0.8f, 1.0f));
        }
        if (TimeGUI::Button(label, TEVector2(-1, 32)))
        {
            mode->m_ActiveVectorTool = type;
        }
        if (active)
        {
            TimeGUI::PopStyleColor();
        }
    };

    ToolBtn("Selection (V)", VectorShapeType::Selection);
    ToolBtn("Pen Tool (P)", VectorShapeType::Pen);
    ToolBtn("Rectangle (R)", VectorShapeType::Rectangle);
    ToolBtn("Circle (C)", VectorShapeType::Circle);
    ToolBtn("Triangle (T)", VectorShapeType::Triangle);
    ToolBtn("Semicircle", VectorShapeType::Semicircle);

    TimeGUI::Spacing();
    TimeGUI::Separator();
    TimeGUI::Text("Drawing Colors");

    TimeGUI::Text("Fill Color");
    TimeGUI::ColorEdit4("##Fill", (float *)&mode->m_ActiveFillColor);

    TimeGUI::Text("Stroke Color");
    TimeGUI::ColorEdit4("##Stroke", (float *)&mode->m_ActiveStrokeColor);

    TimeGUI::Text("Thickness");
    TimeGUI::DragFloat("##Thick", &mode->m_ActiveStrokeThickness, 0.1f, 0.0f, 64.0f);

    TimeGUI::Checkbox("Subtract (Cutout)", &mode->m_DefaultSubtract);

    TimeGUI::EndChild();
}

void VectorEditorSubmode::DrawVectorCanvas(SpriteMode *mode)
{
    TimeGUI::BeginChild("##VectorCanvasViewport", TEVector2(0, 0), true);
    TEVector2 canvasPos = TimeGUI::GetCursorScreenPos();
    TEVector2 canvasSize = TimeGUI::GetContentRegionAvail();

    TimeGUI::TimeGUIDrawList dl = TimeGUI::GetWindowDrawList();
    dl.AddRectFilled(canvasPos, TEVector2(canvasPos.x + canvasSize.x, canvasPos.y + canvasSize.y),
                     IM_COL32(25, 25, 30, 255));

    // Checkerboard Background
    float checkSize = 16.0f;
    for (float y = 0; y < canvasSize.y; y += checkSize)
    {
        for (float x = 0; x < canvasSize.x; x += checkSize)
        {
            int ix = (int)(x / checkSize);
            int iy = (int)(y / checkSize);
            ImU32 col = ((ix + iy) % 2 == 0) ? IM_COL32(30, 30, 35, 255) : IM_COL32(40, 40, 45, 255);
            dl.AddRectFilled(TEVector2(canvasPos.x + x, canvasPos.y + y),
                             TEVector2(canvasPos.x + std::min(x + checkSize, canvasSize.x),
                                       canvasPos.y + std::min(y + checkSize, canvasSize.y)),
                             col);
        }
    }

    // Render Vector Shapes onto the canvas
    mode->RenderVectorShapes(dl, canvasPos, canvasSize, mode->m_CanvasZoom, mode->m_CanvasPan, -1,
                             mode->m_SelectedElementIdx);

    // Canvas Interactions (Pan / Zoom)
    bool hovered = TimeGUI::IsWindowHovered();
    if (hovered)
    {
        float wheel = TimeGUI::GetIO().MouseWheel;
        if (wheel != 0.0f)
        {
            mode->m_CanvasZoom = std::clamp(mode->m_CanvasZoom + wheel * 0.1f, 0.1f, 10.0f);
        }
        if (TimeGUI::IsMouseDragging(TimeGUIMouseButton_Right) || TimeGUI::IsMouseDragging(TimeGUIMouseButton_Middle))
        {
            mode->m_CanvasPan.x += TimeGUI::GetIO().MouseDelta.x / mode->m_CanvasZoom;
            mode->m_CanvasPan.y += TimeGUI::GetIO().MouseDelta.y / mode->m_CanvasZoom;
        }
    }

    TimeGUI::EndChild();
}

void VectorEditorSubmode::DrawPropertiesSidebar(SpriteMode *mode)
{
    TimeGUI::BeginChild("##VecProps", TEVector2(0, 0), true);
    TimeGUI::TextColored(TEVector4(0.4f, 0.8f, 1.0f, 1.0f), "SHAPE HIERARCHY");
    TimeGUI::Separator();

    if (TimeGUI::Button("Clear Shapes", TEVector2(-1, 26)))
    {
        mode->m_VectorElements.clear();
        mode->m_SelectedElementIdx = -1;
        mode->SaveUndoState();
    }
    TimeGUI::Separator();

    for (int i = (int)mode->m_VectorElements.size() - 1; i >= 0; i--)
    {
        TimeGUI::PushID(i);
        bool selected = (mode->m_SelectedElementIdx == i);
        TEString label = "Shape #" + TEString::FromInt(i);
        if (TimeGUI::Selectable(label.c_str(), selected))
        {
            mode->m_SelectedElementIdx = i;
        }
        TimeGUI::PopID();
    }

    TimeGUI::EndChild();
}

