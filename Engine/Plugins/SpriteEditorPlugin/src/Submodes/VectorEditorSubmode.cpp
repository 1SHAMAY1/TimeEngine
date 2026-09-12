#include "VectorEditorSubmode.hpp"
#include "../SpriteEditorLayer.hpp"
#include "../SpriteMode.hpp"
#include "Input/Input.hpp"
#include "Utils/TimeGUI.hpp"
#include <algorithm>
#include <cmath>

void VectorEditorSubmode::OnEnter(SpriteMode *mode)
{
    if (mode && mode->m_VectorFrames.IsEmpty())
    {
        VectorFrame frame;
        mode->m_VectorFrames.Add(frame);
        mode->m_ActiveVectorFrameIndex = 0;
    }
}

void VectorEditorSubmode::OnUpdate(float dt, SpriteMode *mode)
{
    if (!mode || mode->m_VectorFrames.IsEmpty())
        return;

    if (m_IsPlayingPreview && m_PreviewFPS > 0)
    {
        m_AnimationTimer += dt;
        float frameDuration = 1.0f / (float)m_PreviewFPS;
        if (m_AnimationTimer >= frameDuration)
        {
            m_AnimationTimer -= frameDuration;
            m_PreviewFrameIndex = (m_PreviewFrameIndex + 1) % (int)mode->m_VectorFrames.Size();
        }
    }
}

void VectorEditorSubmode::OnExit(SpriteMode *mode)
{
    m_IsInteracting = false;
    m_PenCurrentPath.Clear();
}

bool VectorEditorSubmode::OnShortcut(const TEString &shortcutId, SpriteMode *mode)
{
    if (!mode)
        return false;

    if (shortcutId == "Vector_Select")
    {
        mode->m_ActiveVectorTool = VectorShapeType::Selection;
        return true;
    }
    if (shortcutId == "Vector_Pen")
    {
        mode->m_ActiveVectorTool = VectorShapeType::Pen;
        return true;
    }
    if (shortcutId == "Vector_Rect")
    {
        mode->m_ActiveVectorTool = VectorShapeType::Rectangle;
        return true;
    }
    if (shortcutId == "Vector_Circle")
    {
        mode->m_ActiveVectorTool = VectorShapeType::Circle;
        return true;
    }
    if (shortcutId == "Vector_Triangle")
    {
        mode->m_ActiveVectorTool = VectorShapeType::Triangle;
        return true;
    }
    return false;
}

void VectorEditorSubmode::OnTimeGUIRender(SpriteEditorLayer *layer, SpriteMode *mode)
{
    if (!mode)
        return;

    if (mode->m_VectorFrames.IsEmpty())
    {
        VectorFrame frame;
        mode->m_VectorFrames.Add(frame);
        mode->m_ActiveVectorFrameIndex = 0;
    }

    if (TimeGUI::BeginTable("##VectorStudioLayout", 4, TimeGUITableFlags_Resizable | TimeGUITableFlags_BordersInnerV))
    {
        TimeGUI::TableSetupColumn("Tools", TimeGUITableColumnFlags_WidthFixed, 175.0f);
        TimeGUI::TableSetupColumn("Frames", TimeGUITableColumnFlags_WidthFixed, 130.0f);
        TimeGUI::TableSetupColumn("Canvas", TimeGUITableColumnFlags_WidthStretch, 0.6f);
        TimeGUI::TableSetupColumn("Properties", TimeGUITableColumnFlags_WidthFixed, 230.0f);

        // 1. Tool Sidebar
        TimeGUI::TableNextColumn();
        DrawToolSidebar(mode);

        // 2. Animation Frame Strip
        TimeGUI::TableNextColumn();
        DrawAnimationFrameStrip(mode);

        // 3. Vector Canvas Viewport
        TimeGUI::TableNextColumn();
        DrawVectorCanvas(mode);

        // 4. Properties & Shape Hierarchy Sidebar
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

    float availW = TimeGUI::GetContentRegionAvail().x;
    float btnW = Max(56.0f, (availW - 8.0f) * 0.5f);

    auto ToolBtn = [&](const TEString &label, VectorShapeType type, bool isFullWidth = false)
    {
        bool active = (mode->m_ActiveVectorTool == type);
        if (active)
            TimeGUI::PushStyleColor(TimeGUICol_Button, TEVector4(0.20f, 0.45f, 0.85f, 1.0f));

        if (TimeGUI::Button(label, TEVector2(isFullWidth ? -1 : btnW, 34)))
        {
            mode->m_ActiveVectorTool = type;
            m_PenCurrentPath.Clear();
        }

        if (active)
            TimeGUI::PopStyleColor();
    };

    ToolBtn("Select", VectorShapeType::Selection);
    TimeGUI::SameLine();
    ToolBtn("Pen", VectorShapeType::Pen);

    ToolBtn("Rect", VectorShapeType::Rectangle);
    TimeGUI::SameLine();
    ToolBtn("Circle", VectorShapeType::Circle);

    ToolBtn("Triangle", VectorShapeType::Triangle);
    TimeGUI::SameLine();
    ToolBtn("SemiCir", VectorShapeType::Semicircle);

    TimeGUI::Spacing();
    TimeGUI::Separator();
    TimeGUI::TextColored(TEVector4(1.0f, 0.8f, 0.4f, 1.0f), "STYLE & STROKE");

    TimeGUI::Text("Fill Color");
    TimeGUI::ColorEdit4("##Fill", (float *)&mode->m_ActiveFillColor,
                        TimeGUIColorEditFlags_NoInputs | TimeGUIColorEditFlags_AlphaBar);

    TimeGUI::Text("Stroke Color");
    TimeGUI::ColorEdit4("##Stroke", (float *)&mode->m_ActiveStrokeColor,
                        TimeGUIColorEditFlags_NoInputs | TimeGUIColorEditFlags_AlphaBar);

    TimeGUI::Text("Thickness");
    TimeGUI::SetNextItemWidth(-1);
    TimeGUI::DragFloat("##Thick", &mode->m_ActiveStrokeThickness, 0.1f, 0.0f, 32.0f, "%.1f px");

    TimeGUI::Checkbox("Subtract (Cutout)", &mode->m_DefaultSubtract);

    TimeGUI::Spacing();
    TimeGUI::Separator();
    TimeGUI::Checkbox("Onion Skin", &m_EnableOnionSkin);
    if (m_EnableOnionSkin)
    {
        TimeGUI::Indent(10.0f);
        TimeGUI::SliderInt("Past", &m_OnionPastFrames, 0, 5);
        TimeGUI::SliderInt("Future", &m_OnionFutureFrames, 0, 5);
        TimeGUI::SliderFloat("Ghost Alpha", &m_OnionOpacity, 0.05f, 0.80f, "%.2f");
        TimeGUI::Unindent(10.0f);
    }

    TimeGUI::EndChild();
}

void VectorEditorSubmode::DrawAnimationFrameStrip(SpriteMode *mode)
{
    TimeGUI::BeginChild("##VecFrameStripPane", TEVector2(0, 0), true);
    TimeGUI::TextColored(TEVector4(0.4f, 0.8f, 1.0f, 1.0f), "FRAMES");
    TimeGUI::Separator();

    if (TimeGUI::Button("+ Add Frame", TEVector2(-1, 28)))
    {
        VectorFrame newFrame;
        mode->m_VectorFrames.Add(newFrame);
        mode->m_ActiveVectorFrameIndex = (int)mode->m_VectorFrames.Size() - 1;
        mode->SaveUndoState();
    }

    if (TimeGUI::Button("Duplicate", TEVector2(-1, 24)))
    {
        if (mode->m_ActiveVectorFrameIndex >= 0 && mode->m_ActiveVectorFrameIndex < (int)mode->m_VectorFrames.Size())
        {
            VectorFrame copy = mode->m_VectorFrames[mode->m_ActiveVectorFrameIndex];
            mode->m_VectorFrames.Insert(mode->m_ActiveVectorFrameIndex + 1, copy);
            mode->m_ActiveVectorFrameIndex++;
            mode->SaveUndoState();
        }
    }

    TimeGUI::Separator();

    // Frame Cards Stack
    for (int i = 0; i < (int)mode->m_VectorFrames.Size(); i++)
    {
        TimeGUI::PushID(i);
        bool isCurrent = (mode->m_ActiveVectorFrameIndex == i);

        TEString label = "Frame " + TEString::FromInt(i + 1);
        if (isCurrent)
            TimeGUI::PushStyleColor(TimeGUICol_Button, TEVector4(0.18f, 0.45f, 0.85f, 1.0f));

        if (TimeGUI::Button(label.c_str(), TEVector2(-32, 42)))
        {
            mode->m_ActiveVectorFrameIndex = i;
            mode->m_SelectedElementIdx = -1;
        }

        if (isCurrent)
            TimeGUI::PopStyleColor();

        TimeGUI::SameLine();
        if (TimeGUI::Button("X", TEVector2(26, 42)))
        {
            if (mode->m_VectorFrames.Size() > 1)
            {
                mode->m_VectorFrames.RemoveAt(i);
                if (mode->m_ActiveVectorFrameIndex >= (int)mode->m_VectorFrames.Size())
                    mode->m_ActiveVectorFrameIndex = (int)mode->m_VectorFrames.Size() - 1;
                mode->m_SelectedElementIdx = -1;
                mode->SaveUndoState();
                TimeGUI::PopID();
                break;
            }
        }

        TimeGUI::PopID();
    }

    TimeGUI::EndChild();
}

void VectorEditorSubmode::DrawVectorCanvas(SpriteMode *mode)
{
    TimeGUI::BeginChild("##VectorCanvasViewport", TEVector2(0, 0), true);
    TEVector2 canvasPos = TimeGUI::GetCursorScreenPos();
    TEVector2 canvasSize = TimeGUI::GetContentRegionAvail();
    if (canvasSize.x <= 1.0f || canvasSize.y <= 1.0f)
    {
        TimeGUI::EndChild();
        return;
    }

    TimeGUI::TimeGUIDrawList dl = TimeGUI::GetWindowDrawList();
    dl.PushClipRect(canvasPos, TEVector2(canvasPos.x + canvasSize.x, canvasPos.y + canvasSize.y), true);
    dl.AddRectFilled(canvasPos, TEVector2(canvasPos.x + canvasSize.x, canvasPos.y + canvasSize.y),
                     IM_COL32(20, 20, 25, 255));

    // Checkerboard Background
    float checkSize = 16.0f;
    for (float y = 0; y < canvasSize.y; y += checkSize)
    {
        for (float x = 0; x < canvasSize.x; x += checkSize)
        {
            int ix = (int)(x / checkSize);
            int iy = (int)(y / checkSize);
            unsigned int col = ((ix + iy) % 2 == 0) ? IM_COL32(28, 28, 34, 255) : IM_COL32(36, 36, 44, 255);
            dl.AddRectFilled(TEVector2(canvasPos.x + x, canvasPos.y + y),
                             TEVector2(canvasPos.x + Min(x + checkSize, canvasSize.x),
                                       canvasPos.y + Min(y + checkSize, canvasSize.y)),
                             col);
        }
    }

    // Render Vector Onion Skinning (Past & Future Frame Outlines)
    if (m_EnableOnionSkin && mode->m_VectorFrames.Size() > 1)
    {
        int curF = mode->m_ActiveVectorFrameIndex;
        int totalF = (int)mode->m_VectorFrames.Size();

        // Past vector frames (warm tint)
        for (int p = 1; p <= m_OnionPastFrames; ++p)
        {
            int pastIdx = curF - p;
            if (pastIdx < 0)
                break;
            mode->RenderVectorShapes(dl, canvasPos, canvasSize, mode->m_CanvasZoom, mode->m_CanvasPan, -1, -1, pastIdx);
        }
        // Future vector frames (cool tint)
        for (int f = 1; f <= m_OnionFutureFrames; ++f)
        {
            int futureIdx = curF + f;
            if (futureIdx >= totalF)
                break;
            mode->RenderVectorShapes(dl, canvasPos, canvasSize, mode->m_CanvasZoom, mode->m_CanvasPan, -1, -1,
                                     futureIdx);
        }
    }

    // Render committed vector shapes
    mode->RenderVectorShapes(dl, canvasPos, canvasSize, mode->m_CanvasZoom, mode->m_CanvasPan, -1,
                             mode->m_SelectedElementIdx, mode->m_ActiveVectorFrameIndex);

    // Interactive Drawing Handlers
    bool hovered = TimeGUI::IsWindowHovered();
    TEVector2 mousePos = TimeGUI::GetMousePos();

    // Convert mouse screen position to normalized canvas coordinate [0, 1]
    TEVector2 normPos = TEVector2(
        (mousePos.x - canvasPos.x - mode->m_CanvasPan.x * mode->m_CanvasZoom) / (canvasSize.x * mode->m_CanvasZoom),
        (mousePos.y - canvasPos.y - mode->m_CanvasPan.y * mode->m_CanvasZoom) / (canvasSize.y * mode->m_CanvasZoom));

    auto GetCurrentElements = [&]() -> TEArray<VectorElement> *
    {
        if (mode->m_ActiveVectorFrameIndex >= 0 && mode->m_ActiveVectorFrameIndex < (int)mode->m_VectorFrames.Size())
        {
            return &mode->m_VectorFrames[mode->m_ActiveVectorFrameIndex].Elements;
        }
        return nullptr;
    };

    if (hovered)
    {
        // Pan & Zoom
        float wheel = TimeGUI::GetIO().MouseWheel;
        if (wheel != 0.0f)
        {
            mode->m_CanvasZoom = Clamp(mode->m_CanvasZoom + wheel * 0.1f, 0.1f, 10.0f);
        }
        if (TimeGUI::IsMouseDragging(TimeGUIMouseButton_Right) || TimeGUI::IsMouseDragging(TimeGUIMouseButton_Middle))
        {
            mode->m_CanvasPan.x += TimeGUI::GetIO().MouseDelta.x / mode->m_CanvasZoom;
            mode->m_CanvasPan.y += TimeGUI::GetIO().MouseDelta.y / mode->m_CanvasZoom;
        }

        // Left Click Drawing
        if (mode->m_ActiveVectorTool == VectorShapeType::Rectangle ||
            mode->m_ActiveVectorTool == VectorShapeType::Circle ||
            mode->m_ActiveVectorTool == VectorShapeType::Triangle ||
            mode->m_ActiveVectorTool == VectorShapeType::Semicircle)
        {
            if (TimeGUI::IsMouseClicked(TimeGUIMouseButton_Left))
            {
                m_IsInteracting = true;
                m_StartNormPos = normPos;
            }
        }
        else if (mode->m_ActiveVectorTool == VectorShapeType::Pen)
        {
            if (TimeGUI::IsMouseClicked(TimeGUIMouseButton_Left))
            {
                m_PenCurrentPath.Add(normPos);
            }
            if (TimeGUI::IsMouseClicked(TimeGUIMouseButton_Right))
            {
                if (m_PenCurrentPath.Size() >= 2)
                {
                    auto *elements = GetCurrentElements();
                    if (elements)
                    {
                        VectorElement elem;
                        elem.Type = VectorShapeType::Pen;
                        elem.Points = m_PenCurrentPath;
                        elem.FillColor = mode->m_ActiveFillColor;
                        elem.StrokeColor = mode->m_ActiveStrokeColor;
                        elem.StrokeThickness = mode->m_ActiveStrokeThickness;
                        elem.Subtract = mode->m_DefaultSubtract;
                        elements->Add(elem);
                        mode->m_SelectedElementIdx = (int)elements->Size() - 1;
                        mode->SaveUndoState();
                    }
                }
                m_PenCurrentPath.Clear();
            }
        }
        else if (mode->m_ActiveVectorTool == VectorShapeType::Selection)
        {
            if (TimeGUI::IsMouseClicked(TimeGUIMouseButton_Left))
            {
                auto *elements = GetCurrentElements();
                if (elements)
                {
                    mode->m_SelectedElementIdx = -1;
                    for (int i = (int)elements->Size() - 1; i >= 0; i--)
                    {
                        const auto &elem = (*elements)[i];
                        if (!elem.Points.IsEmpty())
                        {
                            float dist = std::hypot(elem.Points[0].x - normPos.x, elem.Points[0].y - normPos.y);
                            if (dist < 0.1f)
                            {
                                mode->m_SelectedElementIdx = i;
                                break;
                            }
                        }
                    }
                }
            }
        }
    }

    // Cancel drawing or delete selected shape on keyboard shortcut
    if (TimeGUI::IsKeyPressed(TimeGUIKey_Escape))
    {
        m_IsInteracting = false;
        m_PenCurrentPath.Clear();
    }
    if ((TimeGUI::IsKeyPressed(TimeGUIKey_Delete) || TimeGUI::IsKeyPressed(TimeGUIKey_Backspace)) &&
        mode->m_SelectedElementIdx >= 0)
    {
        auto *elements = GetCurrentElements();
        if (elements && mode->m_SelectedElementIdx < (int)elements->Size())
        {
            elements->RemoveAt(mode->m_SelectedElementIdx);
            if (mode->m_SelectedElementIdx >= (int)elements->Size())
                mode->m_SelectedElementIdx = (int)elements->Size() - 1;
            mode->SaveUndoState();
        }
    }

    // Live preview while dragging shape
    if (m_IsInteracting)
    {
        if (TimeGUI::IsMouseDown(TimeGUIMouseButton_Left))
        {
            VectorElement previewElem;
            previewElem.Type = mode->m_ActiveVectorTool;
            previewElem.FillColor = mode->m_ActiveFillColor;
            previewElem.StrokeColor = mode->m_ActiveStrokeColor;
            previewElem.StrokeThickness = mode->m_ActiveStrokeThickness;
            previewElem.Subtract = mode->m_DefaultSubtract;

            if (previewElem.Type == VectorShapeType::Rectangle)
            {
                previewElem.Points.Add(m_StartNormPos);
                previewElem.Points.Add(normPos);
            }
            else if (previewElem.Type == VectorShapeType::Circle || previewElem.Type == VectorShapeType::Semicircle)
            {
                previewElem.Points.Add(m_StartNormPos);
                previewElem.Radius = std::hypot(normPos.x - m_StartNormPos.x, normPos.y - m_StartNormPos.y);
            }
            else if (previewElem.Type == VectorShapeType::Triangle)
            {
                previewElem.Points.Add(TEVector2((m_StartNormPos.x + normPos.x) * 0.5f, m_StartNormPos.y));
                previewElem.Points.Add(TEVector2(m_StartNormPos.x, normPos.y));
                previewElem.Points.Add(normPos);
            }

            unsigned int fillCol = TimeGUI::ColorConvertFloat4ToU32(previewElem.FillColor);
            unsigned int strokeCol = TimeGUI::ColorConvertFloat4ToU32(previewElem.StrokeColor);

            if (previewElem.Type == VectorShapeType::Rectangle && previewElem.Points.Size() >= 2)
            {
                TEVector2 p1 = TEVector2(
                    canvasPos.x + (previewElem.Points[0].x * canvasSize.x + mode->m_CanvasPan.x) * mode->m_CanvasZoom,
                    canvasPos.y + (previewElem.Points[0].y * canvasSize.y + mode->m_CanvasPan.y) * mode->m_CanvasZoom);
                TEVector2 p2 = TEVector2(
                    canvasPos.x + (previewElem.Points[1].x * canvasSize.x + mode->m_CanvasPan.x) * mode->m_CanvasZoom,
                    canvasPos.y + (previewElem.Points[1].y * canvasSize.y + mode->m_CanvasPan.y) * mode->m_CanvasZoom);
                if (previewElem.FillColor.w > 0.0f)
                    dl.AddRectFilled(p1, p2, fillCol);
                if (previewElem.StrokeThickness > 0.0f)
                    dl.AddRect(p1, p2, strokeCol, 0.0f, 0, (float)(previewElem.StrokeThickness * mode->m_CanvasZoom));
            }
            else if ((previewElem.Type == VectorShapeType::Circle || previewElem.Type == VectorShapeType::Semicircle) &&
                     !previewElem.Points.IsEmpty())
            {
                TEVector2 center = TEVector2(
                    canvasPos.x + (previewElem.Points[0].x * canvasSize.x + mode->m_CanvasPan.x) * mode->m_CanvasZoom,
                    canvasPos.y + (previewElem.Points[0].y * canvasSize.y + mode->m_CanvasPan.y) * mode->m_CanvasZoom);
                float r = previewElem.Radius * canvasSize.x * mode->m_CanvasZoom;
                if (previewElem.FillColor.w > 0.0f)
                    dl.AddCircleFilled(center, r, fillCol);
                if (previewElem.StrokeThickness > 0.0f)
                    dl.AddCircle(center, r, strokeCol, 0, (float)(previewElem.StrokeThickness * mode->m_CanvasZoom));
            }
            else if (previewElem.Type == VectorShapeType::Triangle && previewElem.Points.Size() >= 3)
            {
                TEVector2 p1 = TEVector2(
                    canvasPos.x + (previewElem.Points[0].x * canvasSize.x + mode->m_CanvasPan.x) * mode->m_CanvasZoom,
                    canvasPos.y + (previewElem.Points[0].y * canvasSize.y + mode->m_CanvasPan.y) * mode->m_CanvasZoom);
                TEVector2 p2 = TEVector2(
                    canvasPos.x + (previewElem.Points[1].x * canvasSize.x + mode->m_CanvasPan.x) * mode->m_CanvasZoom,
                    canvasPos.y + (previewElem.Points[1].y * canvasSize.y + mode->m_CanvasPan.y) * mode->m_CanvasZoom);
                TEVector2 p3 = TEVector2(
                    canvasPos.x + (previewElem.Points[2].x * canvasSize.x + mode->m_CanvasPan.x) * mode->m_CanvasZoom,
                    canvasPos.y + (previewElem.Points[2].y * canvasSize.y + mode->m_CanvasPan.y) * mode->m_CanvasZoom);
                if (previewElem.FillColor.w > 0.0f)
                    dl.AddTriangleFilled(p1, p2, p3, fillCol);
                if (previewElem.StrokeThickness > 0.0f)
                {
                    TEVector2 triPts[4] = {p1, p2, p3, p1};
                    dl.AddPolyline(triPts, 4, strokeCol, 0, (float)(previewElem.StrokeThickness * mode->m_CanvasZoom));
                }
            }
        }
        else if (TimeGUI::IsMouseReleased(TimeGUIMouseButton_Left))
        {
            m_IsInteracting = false;
            auto *elements = GetCurrentElements();
            if (elements)
            {
                VectorElement newElem;
                newElem.Type = mode->m_ActiveVectorTool;
                newElem.FillColor = mode->m_ActiveFillColor;
                newElem.StrokeColor = mode->m_ActiveStrokeColor;
                newElem.StrokeThickness = mode->m_ActiveStrokeThickness;
                newElem.Subtract = mode->m_DefaultSubtract;

                if (newElem.Type == VectorShapeType::Rectangle)
                {
                    newElem.Points.Add(m_StartNormPos);
                    newElem.Points.Add(normPos);
                }
                else if (newElem.Type == VectorShapeType::Circle || newElem.Type == VectorShapeType::Semicircle)
                {
                    newElem.Points.Add(m_StartNormPos);
                    newElem.Radius = std::hypot(normPos.x - m_StartNormPos.x, normPos.y - m_StartNormPos.y);
                }
                else if (newElem.Type == VectorShapeType::Triangle)
                {
                    newElem.Points.Add(TEVector2((m_StartNormPos.x + normPos.x) * 0.5f, m_StartNormPos.y));
                    newElem.Points.Add(TEVector2(m_StartNormPos.x, normPos.y));
                    newElem.Points.Add(normPos);
                }

                elements->Add(newElem);
                mode->m_SelectedElementIdx = (int)elements->Size() - 1;
                mode->SaveUndoState();
            }
        }
    }

    // Active Pen Path Drawing (Only if Pen tool is actively selected)
    if (mode->m_ActiveVectorTool == VectorShapeType::Pen && !m_PenCurrentPath.IsEmpty())
    {
        TEArray<TEVector2> screenPts;
        screenPts.Reserve(m_PenCurrentPath.Size() + 1);
        for (const auto &pt : m_PenCurrentPath)
        {
            screenPts.Add(TEVector2(canvasPos.x + (pt.x * canvasSize.x + mode->m_CanvasPan.x) * mode->m_CanvasZoom,
                                    canvasPos.y + (pt.y * canvasSize.y + mode->m_CanvasPan.y) * mode->m_CanvasZoom));
        }
        if (hovered)
        {
            screenPts.Add(mousePos);
        }

        dl.AddPolyline(screenPts.Data(), (int)screenPts.Size(), IM_COL32(0, 200, 255, 255), 0, 2.0f);
        for (const auto &sp : screenPts)
        {
            dl.AddCircleFilled(sp, 3.5f, IM_COL32(255, 255, 255, 255));
        }
    }

    dl.PopClipRect();
    TimeGUI::EndChild();
}

void VectorEditorSubmode::DrawPropertiesSidebar(SpriteMode *mode)
{
    TimeGUI::BeginChild("##VecProps", TEVector2(0, 0), true);

    // 1. Live Animated Preview
    TimeGUI::TextColored(TEVector4(0.4f, 0.8f, 1.0f, 1.0f), "PREVIEW");
    TimeGUI::Separator();

    TEVector2 previewPos = TimeGUI::GetCursorScreenPos();
    TimeGUI::TimeGUIDrawList dl = TimeGUI::GetWindowDrawList();
    float previewBox = 72.0f;

    dl.PushClipRect(previewPos, TEVector2(previewPos.x + previewBox, previewPos.y + previewBox), true);
    dl.AddRectFilled(previewPos, TEVector2(previewPos.x + previewBox, previewPos.y + previewBox),
                     IM_COL32(15, 15, 20, 255), 4.0f);

    if (!mode->m_VectorFrames.IsEmpty() && m_PreviewFrameIndex >= 0 &&
        m_PreviewFrameIndex < (int)mode->m_VectorFrames.Size())
    {
        mode->RenderVectorShapes(dl, previewPos, TEVector2(previewBox, previewBox), 1.0f, TEVector2(0, 0), -1, -1,
                                 m_PreviewFrameIndex);
    }
    dl.PopClipRect();

    TimeGUI::Dummy(TEVector2(previewBox, previewBox));
    TimeGUI::SameLine(previewBox + 16.0f);

    TimeGUI::BeginGroup();
    TimeGUI::SetNextItemWidth(90);
    TimeGUI::SliderInt("FPS", &m_PreviewFPS, 1, 30);

    if (TimeGUI::Button(m_IsPlayingPreview ? "Pause" : "Play", TEVector2(70, 24)))
    {
        m_IsPlayingPreview = !m_IsPlayingPreview;
    }
    TimeGUI::EndGroup();

    TimeGUI::Spacing();
    TimeGUI::Separator();

    // 2. Shape Hierarchy
    TimeGUI::TextColored(TEVector4(0.4f, 0.8f, 1.0f, 1.0f), "SHAPE HIERARCHY");
    TimeGUI::Separator();

    auto *elements =
        (mode->m_ActiveVectorFrameIndex >= 0 && mode->m_ActiveVectorFrameIndex < (int)mode->m_VectorFrames.Size())
            ? &mode->m_VectorFrames[mode->m_ActiveVectorFrameIndex].Elements
            : nullptr;

    if (TimeGUI::Button("Clear Shapes", TEVector2(-1, 26)) && elements)
    {
        elements->Clear();
        mode->m_SelectedElementIdx = -1;
        mode->SaveUndoState();
    }
    TimeGUI::Separator();

    if (elements)
    {
        for (int i = (int)elements->Size() - 1; i >= 0; i--)
        {
            TimeGUI::PushID(i);
            bool selected = (mode->m_SelectedElementIdx == i);
            auto &elem = (*elements)[i];

            const char *shapeName = "Shape";
            if (elem.Type == VectorShapeType::Rectangle)
                shapeName = "Rectangle";
            else if (elem.Type == VectorShapeType::Circle)
                shapeName = "Circle";
            else if (elem.Type == VectorShapeType::Triangle)
                shapeName = "Triangle";
            else if (elem.Type == VectorShapeType::Semicircle)
                shapeName = "Semicircle";
            else if (elem.Type == VectorShapeType::Pen)
                shapeName = "Pen Path";

            TEString label = TEString(shapeName) + " #" + TEString::FromInt(i + 1);

            float totalRowW = TimeGUI::GetContentRegionAvail().x;
            if (TimeGUI::Selectable(label.c_str(), selected, 0, TEVector2(totalRowW - 30, 22)))
            {
                mode->m_SelectedElementIdx = i;
            }

            TimeGUI::SameLine();
            if (TimeGUI::Button("X", TEVector2(24, 22)))
            {
                elements->RemoveAt(i);
                if (mode->m_SelectedElementIdx >= (int)elements->Size())
                    mode->m_SelectedElementIdx = (int)elements->Size() - 1;
                mode->SaveUndoState();
                TimeGUI::PopID();
                break;
            }

            TimeGUI::PopID();
        }
    }

    TimeGUI::EndChild();
}
