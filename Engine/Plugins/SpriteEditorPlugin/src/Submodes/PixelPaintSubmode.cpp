#include "PixelPaintSubmode.hpp"
#include "../SpriteEditorLayer.hpp"
#include "../SpriteMode.hpp"
#include "Input/Input.hpp"
#include "Utils/TimeGUI.hpp"
#include <algorithm>
#include <queue>

PixelPaintSubmode::PixelPaintSubmode() {}

void PixelPaintSubmode::OnEnter(SpriteMode *mode)
{
    if (mode && mode->m_PixelFrames.empty())
    {
        PixelFrame frame;
        PixelLayer layer;
        layer.Name = "Layer 1";
        layer.Pixels.resize(mode->m_PixelGridWidth * mode->m_PixelGridHeight, TEVector4(0, 0, 0, 0));
        frame.Layers.push_back(layer);
        mode->m_PixelFrames.push_back(frame);
        mode->m_ActiveFrameIndex = 0;
        mode->m_ActiveLayerIndex = 0;
    }
}

void PixelPaintSubmode::OnUpdate(float dt, SpriteMode *mode)
{
    if (!mode || mode->m_PixelFrames.empty())
        return;

    // Live looping animation player
    if (m_IsPlayingPreview && m_PreviewFPS > 0)
    {
        m_AnimationTimer += dt;
        float frameDuration = 1.0f / (float)m_PreviewFPS;
        if (m_AnimationTimer >= frameDuration)
        {
            m_AnimationTimer -= frameDuration;
            m_PreviewFrameIndex = (m_PreviewFrameIndex + 1) % (int)mode->m_PixelFrames.size();
        }
    }
}

void PixelPaintSubmode::OnExit(SpriteMode *mode) {}

bool PixelPaintSubmode::OnShortcut(const TEString &shortcutId, SpriteMode *mode)
{
    if (!mode)
        return false;

    if (shortcutId == "Pixel_Pencil")
    {
        m_ActiveTool = PixelToolType::Pencil;
        return true;
    }
    if (shortcutId == "Pixel_MirrorPen")
    {
        m_ActiveTool = PixelToolType::MirrorPen;
        return true;
    }
    if (shortcutId == "Pixel_Eraser")
    {
        m_ActiveTool = PixelToolType::Eraser;
        return true;
    }
    if (shortcutId == "Pixel_Bucket")
    {
        m_ActiveTool = PixelToolType::PaintBucket;
        return true;
    }
    if (shortcutId == "Pixel_ColorReplace")
    {
        m_ActiveTool = PixelToolType::ColorReplaceBucket;
        return true;
    }
    if (shortcutId == "Pixel_ColorPicker")
    {
        m_ActiveTool = PixelToolType::ColorPicker;
        return true;
    }
    return false;
}

void PixelPaintSubmode::OnTimeGUIRender(SpriteEditorLayer *layer, SpriteMode *mode)
{
    if (!mode)
        return;

    // Ensure at least one frame & layer exist
    if (mode->m_PixelFrames.empty())
    {
        PixelFrame frame;
        PixelLayer l;
        l.Name = "Layer 1";
        l.Pixels.resize(mode->m_PixelGridWidth * mode->m_PixelGridHeight, TEVector4(0, 0, 0, 0));
        frame.Layers.push_back(l);
        mode->m_PixelFrames.push_back(frame);
        mode->m_ActiveFrameIndex = 0;
        mode->m_ActiveLayerIndex = 0;
    }

    if (TimeGUI::BeginTable("##PiskelStudioLayout", 4, TimeGUITableFlags_Resizable | TimeGUITableFlags_BordersInnerV))
    {
        TimeGUI::TableSetupColumn("Tools", TimeGUITableColumnFlags_WidthFixed, 140.0f);
        TimeGUI::TableSetupColumn("Frames", TimeGUITableColumnFlags_WidthFixed, 130.0f);
        TimeGUI::TableSetupColumn("Canvas", TimeGUITableColumnFlags_WidthStretch, 0.6f);
        TimeGUI::TableSetupColumn("Preview & Layers", TimeGUITableColumnFlags_WidthFixed, 220.0f);

        // 1. Tool Strip (Left)
        TimeGUI::TableNextColumn();
        DrawToolPalette(mode);

        // 2. Animation Frame Strip (Center-Left)
        TimeGUI::TableNextColumn();
        DrawAnimationFrameStrip(mode);

        // 3. Pixel Canvas Viewport (Center)
        TimeGUI::TableNextColumn();
        DrawCanvasViewport(mode);

        // 4. Live Preview, Layers & Transforms (Right)
        TimeGUI::TableNextColumn();
        if (TimeGUI::BeginChild("##RightStudioSidebar", TEVector2(0, 0), false))
        {
            DrawLiveAnimatedPreview(mode);
            DrawLayersPanel(mode);
            DrawTransformPanel(mode);
            DrawPalettesPanel(mode);
        }
        TimeGUI::EndChild();

        TimeGUI::EndTable();
    }

    if (m_ShowResizeDialog)
    {
        DrawResizeModal(mode);
    }
}

void PixelPaintSubmode::DrawToolPalette(SpriteMode *mode)
{
    TimeGUI::BeginChild("##ToolPalettePane", TEVector2(0, 0), true);
    TimeGUI::TextColored(TEVector4(0.4f, 0.8f, 1.0f, 1.0f), "TOOLS");
    TimeGUI::Separator();

    auto ToolBtn = [&](const TEString &label, PixelToolType type, const char *tooltip)
    {
        bool active = (m_ActiveTool == type);
        if (active)
            TimeGUI::PushStyleColor(TimeGUICol_Button, TEVector4(0.20f, 0.45f, 0.85f, 1.0f));

        if (TimeGUI::Button(label, TEVector2(48, 36)))
        {
            m_ActiveTool = type;
        }

        if (active)
            TimeGUI::PopStyleColor();
    };

    // 2-Column Tool Grid
    ToolBtn("Pen", PixelToolType::Pencil, "Pen Tool (P)");
    TimeGUI::SameLine();
    ToolBtn("Mirror", PixelToolType::MirrorPen, "Vertical Mirror Pen (V)");

    ToolBtn("Bucket", PixelToolType::PaintBucket, "Paint Bucket (B)");
    TimeGUI::SameLine();
    ToolBtn("All Col", PixelToolType::ColorReplaceBucket, "Color Replace Bucket (A)");

    ToolBtn("Eraser", PixelToolType::Eraser, "Eraser (E)");
    TimeGUI::SameLine();
    ToolBtn("Dither", PixelToolType::DitheringPen, "Dithering Pen (D)");

    ToolBtn("Line", PixelToolType::Line, "Stroke / Line Tool (L)");
    TimeGUI::SameLine();
    ToolBtn("Picker", PixelToolType::ColorPicker, "Color Picker (I)");

    ToolBtn("Rect", PixelToolType::Rectangle, "Rectangle Tool (U)");
    TimeGUI::SameLine();
    ToolBtn("Circle", PixelToolType::Circle, "Circle Tool (O)");

    TimeGUI::Spacing();
    TimeGUI::Separator();
    TimeGUI::TextColored(TEVector4(1.0f, 0.8f, 0.4f, 1.0f), "PEN SIZE");

    // 1px, 2px, 3px, 4px Brush Sizes
    for (int size = 1; size <= 4; size++)
    {
        TEString label = TEString::FromInt(size) + "px";
        bool active = (m_BrushSize == size);
        if (active)
            TimeGUI::PushStyleColor(TimeGUICol_Button, TEVector4(0.3f, 0.7f, 0.4f, 1.0f));

        if (TimeGUI::Button(label.c_str(), TEVector2(48, 26)))
        {
            m_BrushSize = size;
        }

        if (active)
            TimeGUI::PopStyleColor();

        if (size % 2 != 0)
            TimeGUI::SameLine();
    }

    TimeGUI::Spacing();
    TimeGUI::Separator();

    // Primary & Secondary Swatches with quick-swap
    TimeGUI::Text("Swatches");
    TimeGUI::ColorEdit4("##Pri", (float *)&mode->m_PixelPaintColor,
                        TimeGUIColorEditFlags_NoInputs | TimeGUIColorEditFlags_AlphaBar);
    TimeGUI::SameLine();
    if (TimeGUI::Button("<->", TEVector2(32, 24)))
    {
        std::swap(mode->m_PixelPaintColor, m_SecondaryColor);
    }
    TimeGUI::SameLine();
    TimeGUI::ColorEdit4("##Sec", (float *)&m_SecondaryColor,
                        TimeGUIColorEditFlags_NoInputs | TimeGUIColorEditFlags_AlphaBar);

    TimeGUI::Spacing();
    TimeGUI::Separator();
    TimeGUI::Checkbox("Grid", &m_ShowGrid);
    TimeGUI::Checkbox("Onion Skin", &m_EnableOnionSkin);

    if (TimeGUI::Button("Resize Canvas", TEVector2(-1, 26)))
    {
        m_ShowResizeDialog = true;
        m_NewResizeWidth = mode->m_PixelGridWidth;
        m_NewResizeHeight = mode->m_PixelGridHeight;
    }

    TimeGUI::EndChild();
}

void PixelPaintSubmode::DrawAnimationFrameStrip(SpriteMode *mode)
{
    TimeGUI::BeginChild("##FrameStripPane", TEVector2(0, 0), true);
    TimeGUI::TextColored(TEVector4(0.4f, 0.8f, 1.0f, 1.0f), "FRAMES");
    TimeGUI::Separator();

    if (TimeGUI::Button("+ Add Frame", TEVector2(-1, 28)))
    {
        PixelFrame newFrame;
        PixelLayer newLayer;
        newLayer.Name = "Layer 1";
        newLayer.Pixels.resize(mode->m_PixelGridWidth * mode->m_PixelGridHeight, TEVector4(0, 0, 0, 0));
        newFrame.Layers.push_back(newLayer);
        mode->m_PixelFrames.push_back(newFrame);
        mode->m_ActiveFrameIndex = (int)mode->m_PixelFrames.size() - 1;
        mode->SaveUndoState();
    }

    if (TimeGUI::Button("Duplicate", TEVector2(-1, 24)))
    {
        if (mode->m_ActiveFrameIndex >= 0 && mode->m_ActiveFrameIndex < (int)mode->m_PixelFrames.size())
        {
            PixelFrame copy = mode->m_PixelFrames[mode->m_ActiveFrameIndex];
            mode->m_PixelFrames.insert(mode->m_PixelFrames.begin() + mode->m_ActiveFrameIndex + 1, copy);
            mode->m_ActiveFrameIndex++;
            mode->SaveUndoState();
        }
    }

    TimeGUI::Separator();

    // Frame Cards Stack
    for (int i = 0; i < (int)mode->m_PixelFrames.size(); i++)
    {
        TimeGUI::PushID(i);
        bool isCurrent = (mode->m_ActiveFrameIndex == i);

        TEString label = "Frame " + TEString::FromInt(i + 1);
        if (isCurrent)
            TimeGUI::PushStyleColor(TimeGUICol_Button, TEVector4(0.18f, 0.45f, 0.85f, 1.0f));

        if (TimeGUI::Button(label.c_str(), TEVector2(-32, 42)))
        {
            mode->m_ActiveFrameIndex = i;
        }

        if (isCurrent)
            TimeGUI::PopStyleColor();

        TimeGUI::SameLine();
        if (TimeGUI::Button("X", TEVector2(26, 42)))
        {
            if (mode->m_PixelFrames.size() > 1)
            {
                mode->m_PixelFrames.erase(mode->m_PixelFrames.begin() + i);
                if (mode->m_ActiveFrameIndex >= (int)mode->m_PixelFrames.size())
                    mode->m_ActiveFrameIndex = (int)mode->m_PixelFrames.size() - 1;
                mode->SaveUndoState();
                TimeGUI::PopID();
                break;
            }
        }

        TimeGUI::PopID();
    }

    TimeGUI::EndChild();
}

void PixelPaintSubmode::DrawCanvasViewport(SpriteMode *mode)
{
    TimeGUI::BeginChild("##PixelCanvasPane", TEVector2(0, 0), true);
    TEVector2 canvasPos = TimeGUI::GetCursorScreenPos();
    TEVector2 canvasSize = TimeGUI::GetContentRegionAvail();

    TimeGUI::TimeGUIDrawList dl = TimeGUI::GetWindowDrawList();
    dl.AddRectFilled(canvasPos, TEVector2(canvasPos.x + canvasSize.x, canvasPos.y + canvasSize.y),
                     IM_COL32(18, 18, 22, 255));

    int gridW = mode->m_PixelGridWidth;
    int gridH = mode->m_PixelGridHeight;
    float cellDim = std::min(canvasSize.x / gridW, canvasSize.y / gridH) * mode->m_CanvasZoom;
    TEVector2 origin = TEVector2(canvasPos.x + (canvasSize.x - gridW * cellDim) * 0.5f + mode->m_CanvasPan.x,
                                 canvasPos.y + (canvasSize.y - gridH * cellDim) * 0.5f + mode->m_CanvasPan.y);

    // 1. Checkerboard Background
    float checkSize = std::max(8.0f, cellDim);
    for (float y = 0; y < gridH * cellDim; y += checkSize)
    {
        for (float x = 0; x < gridW * cellDim; x += checkSize)
        {
            int ix = (int)(x / checkSize);
            int iy = (int)(y / checkSize);
            ImU32 col = ((ix + iy) % 2 == 0) ? IM_COL32(26, 26, 30, 255) : IM_COL32(34, 34, 40, 255);
            dl.AddRectFilled(TEVector2(origin.x + x, origin.y + y),
                             TEVector2(origin.x + std::min(x + checkSize, gridW * cellDim),
                                       origin.y + std::min(y + checkSize, gridH * cellDim)),
                             col);
        }
    }

    // 2. Onion Skinning (Ghost Previous Frame)
    if (m_EnableOnionSkin && mode->m_ActiveFrameIndex > 0)
    {
        auto &prevFrame = mode->m_PixelFrames[mode->m_ActiveFrameIndex - 1];
        for (const auto &layer : prevFrame.Layers)
        {
            if (!layer.Visible)
                continue;
            for (int y = 0; y < gridH; y++)
            {
                for (int x = 0; x < gridW; x++)
                {
                    const auto &c = layer.Pixels[y * gridW + x];
                    if (c.w > 0.001f)
                    {
                        TEVector4 ghost = TEVector4(c.x, c.y, c.z, c.w * 0.25f);
                        TEVector2 p1 = TEVector2(origin.x + x * cellDim, origin.y + y * cellDim);
                        TEVector2 p2 = TEVector2(p1.x + cellDim, p1.y + cellDim);
                        dl.AddRectFilled(p1, p2, TimeGUI::ColorConvertFloat4ToU32(ghost));
                    }
                }
            }
        }
    }

    // 3. Composite Visible Layers of Active Frame
    if (mode->m_ActiveFrameIndex >= 0 && mode->m_ActiveFrameIndex < (int)mode->m_PixelFrames.size())
    {
        auto &frame = mode->m_PixelFrames[mode->m_ActiveFrameIndex];
        for (size_t l = 0; l < frame.Layers.size(); l++)
        {
            const auto &layer = frame.Layers[l];
            if (!layer.Visible)
                continue;

            for (int y = 0; y < gridH; y++)
            {
                for (int x = 0; x < gridW; x++)
                {
                    TEVector4 c = layer.Pixels[y * gridW + x];
                    if (c.w > 0.001f)
                    {
                        c.w *= layer.Opacity;
                        TEVector2 p1 = TEVector2(origin.x + x * cellDim, origin.y + y * cellDim);
                        TEVector2 p2 = TEVector2(p1.x + cellDim, p1.y + cellDim);
                        dl.AddRectFilled(p1, p2, TimeGUI::ColorConvertFloat4ToU32(c));
                    }
                }
            }
        }
    }

    // 4. Grid Lines
    if (m_ShowGrid && cellDim >= 6.0f)
    {
        for (int y = 0; y <= gridH; y++)
        {
            dl.AddLine(TEVector2(origin.x, origin.y + y * cellDim),
                       TEVector2(origin.x + gridW * cellDim, origin.y + y * cellDim), IM_COL32(50, 50, 55, 100));
        }
        for (int x = 0; x <= gridW; x++)
        {
            dl.AddLine(TEVector2(origin.x + x * cellDim, origin.y),
                       TEVector2(origin.x + x * cellDim, origin.y + gridH * cellDim), IM_COL32(50, 50, 55, 100));
        }
    }

    // 5. Symmetry Axis Line (for Mirror Pen)
    if (m_ActiveTool == PixelToolType::MirrorPen)
    {
        float midX = origin.x + (gridW * 0.5f) * cellDim;
        dl.AddLine(TEVector2(midX, origin.y), TEVector2(midX, origin.y + gridH * cellDim), IM_COL32(0, 200, 255, 180),
                   1.5f);
    }

    // 6. Interactive Painting
    bool hovered = TimeGUI::IsWindowHovered();
    if (hovered && mode->m_ActiveFrameIndex >= 0 && mode->m_ActiveFrameIndex < (int)mode->m_PixelFrames.size())
    {
        auto &frame = mode->m_PixelFrames[mode->m_ActiveFrameIndex];
        if (mode->m_ActiveLayerIndex >= 0 && mode->m_ActiveLayerIndex < (int)frame.Layers.size())
        {
            auto &layer = frame.Layers[mode->m_ActiveLayerIndex];
            if (!layer.Locked)
            {
                TEVector2 mousePos = TimeGUI::GetMousePos();
                int cellX = (int)((mousePos.x - origin.x) / cellDim);
                int cellY = (int)((mousePos.y - origin.y) / cellDim);

                if (cellX >= 0 && cellX < gridW && cellY >= 0 && cellY < gridH)
                {
                    // Hover Cursor Highlight
                    TEVector2 h1 = TEVector2(origin.x + cellX * cellDim, origin.y + cellY * cellDim);
                    TEVector2 h2 = TEVector2(h1.x + cellDim, h1.y + cellDim);
                    dl.AddRect(h1, h2, IM_COL32(255, 255, 255, 200), 0.0f, 0, 1.5f);

                    if (m_ActiveTool == PixelToolType::MirrorPen)
                    {
                        int mirrorX = gridW - 1 - cellX;
                        TEVector2 m1 = TEVector2(origin.x + mirrorX * cellDim, origin.y + cellY * cellDim);
                        TEVector2 m2 = TEVector2(m1.x + cellDim, m1.y + cellDim);
                        dl.AddRect(m1, m2, IM_COL32(0, 200, 255, 200), 0.0f, 0, 1.5f);
                    }

                    if (TimeGUI::IsMouseDown(TimeGUIMouseButton_Left))
                    {
                        TEVector4 drawCol = mode->m_PixelPaintColor;
                        if (m_ActiveTool == PixelToolType::Pencil)
                        {
                            DrawBrushPixel(layer.Pixels, gridW, gridH, cellX, cellY, drawCol, m_BrushSize);
                            mode->AddColorToHistory(drawCol);
                        }
                        else if (m_ActiveTool == PixelToolType::MirrorPen)
                        {
                            DrawBrushPixel(layer.Pixels, gridW, gridH, cellX, cellY, drawCol, m_BrushSize);
                            DrawBrushPixel(layer.Pixels, gridW, gridH, gridW - 1 - cellX, cellY, drawCol, m_BrushSize);
                            mode->AddColorToHistory(drawCol);
                        }
                        else if (m_ActiveTool == PixelToolType::DitheringPen)
                        {
                            DrawDitherPixel(layer.Pixels, gridW, gridH, cellX, cellY, drawCol, m_BrushSize);
                            mode->AddColorToHistory(drawCol);
                        }
                        else if (m_ActiveTool == PixelToolType::Eraser)
                        {
                            DrawBrushPixel(layer.Pixels, gridW, gridH, cellX, cellY, TEVector4(0, 0, 0, 0),
                                           m_BrushSize);
                        }
                        else if (m_ActiveTool == PixelToolType::PaintBucket &&
                                 TimeGUI::IsMouseClicked(TimeGUIMouseButton_Left))
                        {
                            TEVector4 target = layer.Pixels[cellY * gridW + cellX];
                            FloodFill(layer.Pixels, gridW, gridH, cellX, cellY, target, drawCol);
                            mode->AddColorToHistory(drawCol);
                            mode->SaveUndoState();
                        }
                        else if (m_ActiveTool == PixelToolType::ColorReplaceBucket &&
                                 TimeGUI::IsMouseClicked(TimeGUIMouseButton_Left))
                        {
                            TEVector4 target = layer.Pixels[cellY * gridW + cellX];
                            ReplaceColor(layer.Pixels, gridW, gridH, target, drawCol);
                            mode->AddColorToHistory(drawCol);
                            mode->SaveUndoState();
                        }
                        else if (m_ActiveTool == PixelToolType::ColorPicker &&
                                 TimeGUI::IsMouseClicked(TimeGUIMouseButton_Left))
                        {
                            mode->m_PixelPaintColor = layer.Pixels[cellY * gridW + cellX];
                            m_ActiveTool = PixelToolType::Pencil;
                        }
                    }

                    if (TimeGUI::IsMouseReleased(TimeGUIMouseButton_Left))
                    {
                        mode->SaveUndoState();
                    }
                }
            }
        }

        // Pan & Zoom
        float wheel = TimeGUI::GetIO().MouseWheel;
        if (wheel != 0.0f)
        {
            mode->m_CanvasZoom = std::clamp(mode->m_CanvasZoom + wheel * 0.15f, 0.2f, 20.0f);
        }
        if (TimeGUI::IsMouseDragging(TimeGUIMouseButton_Right) || TimeGUI::IsMouseDragging(TimeGUIMouseButton_Middle))
        {
            mode->m_CanvasPan.x += TimeGUI::GetIO().MouseDelta.x;
            mode->m_CanvasPan.y += TimeGUI::GetIO().MouseDelta.y;
        }
    }

    // Bottom Status Coordinates
    TimeGUI::SetCursorPos(TEVector2(12, canvasSize.y - 28));
    TimeGUI::TextColored(TEVector4(0.8f, 0.8f, 0.8f, 0.9f), "x%.2f [%dx%d] Frame %d/%d", mode->m_CanvasZoom,
                         mode->m_PixelGridWidth, mode->m_PixelGridHeight, mode->m_ActiveFrameIndex + 1,
                         (int)mode->m_PixelFrames.size());

    TimeGUI::EndChild();
}

void PixelPaintSubmode::DrawLiveAnimatedPreview(SpriteMode *mode)
{
    TimeGUI::BeginChild("##PreviewPane", TEVector2(0, 160), true);
    TimeGUI::TextColored(TEVector4(0.4f, 0.8f, 1.0f, 1.0f), "PREVIEW");
    TimeGUI::Separator();

    TEVector2 previewPos = TimeGUI::GetCursorScreenPos();
    TimeGUI::TimeGUIDrawList dl = TimeGUI::GetWindowDrawList();
    float previewBox = 80.0f;

    dl.AddRectFilled(previewPos, TEVector2(previewPos.x + previewBox, previewPos.y + previewBox),
                     IM_COL32(15, 15, 20, 255), 4.0f);

    if (!mode->m_PixelFrames.empty() && m_PreviewFrameIndex >= 0 &&
        m_PreviewFrameIndex < (int)mode->m_PixelFrames.size())
    {
        const auto &frame = mode->m_PixelFrames[m_PreviewFrameIndex];
        int gridW = mode->m_PixelGridWidth;
        int gridH = mode->m_PixelGridHeight;
        float cellDim = previewBox / std::max(gridW, gridH);

        for (const auto &layer : frame.Layers)
        {
            if (!layer.Visible)
                continue;
            for (int y = 0; y < gridH; y++)
            {
                for (int x = 0; x < gridW; x++)
                {
                    TEVector4 c = layer.Pixels[y * gridW + x];
                    if (c.w > 0.001f)
                    {
                        c.w *= layer.Opacity;
                        TEVector2 p1 = TEVector2(previewPos.x + x * cellDim, previewPos.y + y * cellDim);
                        TEVector2 p2 = TEVector2(p1.x + cellDim, p1.y + cellDim);
                        dl.AddRectFilled(p1, p2, TimeGUI::ColorConvertFloat4ToU32(c));
                    }
                }
            }
        }
    }

    TimeGUI::SetCursorPosX(previewBox + 16);
    TimeGUI::SetNextItemWidth(100);
    TimeGUI::SliderInt("FPS", &m_PreviewFPS, 1, 30);

    TimeGUI::SetCursorPosX(previewBox + 16);
    if (TimeGUI::Button(m_IsPlayingPreview ? "Pause" : "Play", TEVector2(60, 24)))
    {
        m_IsPlayingPreview = !m_IsPlayingPreview;
    }

    TimeGUI::EndChild();
}

void PixelPaintSubmode::DrawLayersPanel(SpriteMode *mode)
{
    TimeGUI::BeginChild("##LayersPane", TEVector2(0, 150), true);
    TimeGUI::TextColored(TEVector4(0.4f, 0.8f, 1.0f, 1.0f), "LAYERS");
    TimeGUI::SameLine(TimeGUI::GetWindowWidth() - 36);

    if (mode->m_ActiveFrameIndex >= 0 && mode->m_ActiveFrameIndex < (int)mode->m_PixelFrames.size())
    {
        auto &frame = mode->m_PixelFrames[mode->m_ActiveFrameIndex];

        if (TimeGUI::Button("+", TEVector2(26, 20)))
        {
            PixelLayer newLayer;
            newLayer.Name = "Layer " + TEString::FromInt(static_cast<int>(frame.Layers.size()) + 1);
            newLayer.Pixels.resize(mode->m_PixelGridWidth * mode->m_PixelGridHeight, TEVector4(0, 0, 0, 0));
            frame.Layers.push_back(newLayer);
            mode->m_ActiveLayerIndex = (int)frame.Layers.size() - 1;
            mode->SaveUndoState();
        }

        TimeGUI::Separator();

        for (int l = (int)frame.Layers.size() - 1; l >= 0; l--)
        {
            TimeGUI::PushID(l);
            auto &layer = frame.Layers[l];
            bool active = (mode->m_ActiveLayerIndex == l);

            if (TimeGUI::Checkbox("##Vis", &layer.Visible))
            {
            }
            TimeGUI::SameLine();

            if (TimeGUI::Selectable(layer.Name.c_str(), active, 0, TEVector2(-40, 22)))
            {
                mode->m_ActiveLayerIndex = l;
            }

            TimeGUI::SameLine();
            TimeGUI::SetNextItemWidth(36);
            TimeGUI::DragFloat("##Op", &layer.Opacity, 0.05f, 0.0f, 1.0f, "%.1f");

            TimeGUI::PopID();
        }
    }

    TimeGUI::EndChild();
}

void PixelPaintSubmode::DrawTransformPanel(SpriteMode *mode)
{
    TimeGUI::BeginChild("##TransformPane", TEVector2(0, 110), true);
    TimeGUI::TextColored(TEVector4(0.4f, 0.8f, 1.0f, 1.0f), "TRANSFORM");
    TimeGUI::Separator();

    if (mode->m_ActiveFrameIndex >= 0 && mode->m_ActiveFrameIndex < (int)mode->m_PixelFrames.size())
    {
        auto &frame = mode->m_PixelFrames[mode->m_ActiveFrameIndex];
        if (mode->m_ActiveLayerIndex >= 0 && mode->m_ActiveLayerIndex < (int)frame.Layers.size())
        {
            auto &layer = frame.Layers[mode->m_ActiveLayerIndex];
            int w = mode->m_PixelGridWidth;
            int h = mode->m_PixelGridHeight;

            // Flip Horizontal
            if (TimeGUI::Button("Flip Horiz", TEVector2(80, 24)))
            {
                for (int y = 0; y < h; y++)
                {
                    for (int x = 0; x < w / 2; x++)
                    {
                        std::swap(layer.Pixels[y * w + x], layer.Pixels[y * w + (w - 1 - x)]);
                    }
                }
                mode->SaveUndoState();
            }
            TimeGUI::SameLine();

            // Flip Vertical
            if (TimeGUI::Button("Flip Vert", TEVector2(80, 24)))
            {
                for (int y = 0; y < h / 2; y++)
                {
                    for (int x = 0; x < w; x++)
                    {
                        std::swap(layer.Pixels[y * w + x], layer.Pixels[(h - 1 - y) * w + x]);
                    }
                }
                mode->SaveUndoState();
            }

            // Rotate 90 CW
            if (TimeGUI::Button("Rotate 90", TEVector2(80, 24)) && w == h)
            {
                TEArray<TEVector4> rotated;
                rotated.Resize(w * h, TEVector4(0, 0, 0, 0));
                for (int y = 0; y < h; y++)
                {
                    for (int x = 0; x < w; x++)
                    {
                        rotated[x * w + (w - 1 - y)] = layer.Pixels[y * w + x];
                    }
                }
                layer.Pixels = rotated;
                mode->SaveUndoState();
            }
        }
    }

    TimeGUI::EndChild();
}

void PixelPaintSubmode::DrawPalettesPanel(SpriteMode *mode)
{
    TimeGUI::BeginChild("##PalettePane", TEVector2(0, 0), true);
    TimeGUI::TextColored(TEVector4(0.4f, 0.8f, 1.0f, 1.0f), "PALETTES");
    TimeGUI::Separator();

    for (int i = 0; i < (int)mode->m_ColorHistory.size() && i < 16; i++)
    {
        TimeGUI::PushID(i);
        const auto &col = mode->m_ColorHistory[i];
        if (TimeGUI::ColorButton("##HistSwatch", col, 0, TEVector2(22, 22)))
        {
            mode->m_PixelPaintColor = col;
        }
        if ((i + 1) % 6 != 0 && i + 1 < (int)mode->m_ColorHistory.size())
            TimeGUI::SameLine();
        TimeGUI::PopID();
    }

    TimeGUI::EndChild();
}

void PixelPaintSubmode::DrawResizeModal(SpriteMode *mode)
{
    TimeGUI::OpenPopup("Resize Canvas##Modal");
    if (TimeGUI::BeginPopupModal("Resize Canvas##Modal", &m_ShowResizeDialog, TimeGUIWindowFlags_AlwaysAutoResize))
    {
        TimeGUI::Text("Specify new sprite canvas dimensions:");
        TimeGUI::Separator();

        TimeGUI::InputInt("Width", &m_NewResizeWidth);
        TimeGUI::InputInt("Height", &m_NewResizeHeight);

        m_NewResizeWidth = std::clamp(m_NewResizeWidth, 4, 512);
        m_NewResizeHeight = std::clamp(m_NewResizeHeight, 4, 512);

        TimeGUI::Spacing();
        TimeGUI::Checkbox("Maintain Aspect Ratio", &m_MaintainAspectRatio);
        TimeGUI::Checkbox("Resize Canvas Only (Crop/Expand)", &m_ResizeCanvasOnly);

        TimeGUI::Spacing();
        TimeGUI::Separator();

        if (TimeGUI::Button("Apply", TEVector2(100, 28)))
        {
            int oldW = mode->m_PixelGridWidth;
            int oldH = mode->m_PixelGridHeight;
            int newW = m_NewResizeWidth;
            int newH = m_NewResizeHeight;

            for (auto &frame : mode->m_PixelFrames)
            {
                for (auto &layer : frame.Layers)
                {
                    TEArray<TEVector4> resized;
                    resized.Resize(newW * newH, TEVector4(0, 0, 0, 0));
                    int copyW = std::min(oldW, newW);
                    int copyH = std::min(oldH, newH);

                    for (int y = 0; y < copyH; y++)
                    {
                        for (int x = 0; x < copyW; x++)
                        {
                            resized[y * newW + x] = layer.Pixels[y * oldW + x];
                        }
                    }
                    layer.Pixels = resized;
                }
            }

            mode->m_PixelGridWidth = newW;
            mode->m_PixelGridHeight = newH;
            mode->SaveUndoState();
            m_ShowResizeDialog = false;
        }

        TimeGUI::SameLine();
        if (TimeGUI::Button("Cancel", TEVector2(100, 28)))
        {
            m_ShowResizeDialog = false;
        }

        TimeGUI::EndPopup();
    }
}

void PixelPaintSubmode::DrawBrushPixel(TEArray<TEVector4> &pixels, int width, int height, int x, int y,
                                       const TEVector4 &col, int size)
{
    int half = size / 2;
    for (int dy = -half; dy < size - half; dy++)
    {
        for (int dx = -half; dx < size - half; dx++)
        {
            int px = x + dx;
            int py = y + dy;
            if (px >= 0 && px < width && py >= 0 && py < height)
            {
                pixels[py * width + px] = col;
            }
        }
    }
}

void PixelPaintSubmode::DrawDitherPixel(TEArray<TEVector4> &pixels, int width, int height, int x, int y,
                                        const TEVector4 &col, int size)
{
    int half = size / 2;
    for (int dy = -half; dy < size - half; dy++)
    {
        for (int dx = -half; dx < size - half; dx++)
        {
            int px = x + dx;
            int py = y + dy;
            if (px >= 0 && px < width && py >= 0 && py < height)
            {
                if ((px + py) % 2 == 0)
                {
                    pixels[py * width + px] = col;
                }
            }
        }
    }
}

void PixelPaintSubmode::FloodFill(TEArray<TEVector4> &pixels, int width, int height, int startX, int startY,
                                  const TEVector4 &targetCol, const TEVector4 &fillCol)
{
    if (startX < 0 || startX >= width || startY < 0 || startY >= height)
        return;
    if (std::abs(targetCol.x - fillCol.x) < 0.001f && std::abs(targetCol.y - fillCol.y) < 0.001f &&
        std::abs(targetCol.z - fillCol.z) < 0.001f && std::abs(targetCol.w - fillCol.w) < 0.001f)
        return;

    std::queue<std::pair<int, int>> q;
    q.push({startX, startY});

    while (!q.empty())
    {
        auto [cx, cy] = q.front();
        q.pop();

        if (cx < 0 || cx >= width || cy < 0 || cy >= height)
            continue;

        auto &c = pixels[cy * width + cx];
        if (std::abs(c.x - targetCol.x) < 0.01f && std::abs(c.y - targetCol.y) < 0.01f &&
            std::abs(c.z - targetCol.z) < 0.01f && std::abs(c.w - targetCol.w) < 0.01f)
        {
            c = fillCol;
            q.push({cx + 1, cy});
            q.push({cx - 1, cy});
            q.push({cx, cy + 1});
            q.push({cx, cy - 1});
        }
    }
}

void PixelPaintSubmode::ReplaceColor(TEArray<TEVector4> &pixels, int width, int height, const TEVector4 &targetCol,
                                     const TEVector4 &fillCol)
{
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            auto &c = pixels[y * width + x];
            if (std::abs(c.x - targetCol.x) < 0.01f && std::abs(c.y - targetCol.y) < 0.01f &&
                std::abs(c.z - targetCol.z) < 0.01f && std::abs(c.w - targetCol.w) < 0.01f)
            {
                c = fillCol;
            }
        }
    }
}

void PixelPaintSubmode::DrawBresenhamLine(TEArray<TEVector4> &pixels, int width, int height, int x0, int y0, int x1,
                                          int y1, const TEVector4 &col, int size)
{
}
