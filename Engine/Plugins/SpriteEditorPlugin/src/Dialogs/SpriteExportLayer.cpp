#include "SpriteExportLayer.hpp"
#include "../SpriteMode.hpp"
#include "Core/Asset/AssetManager.hpp"
#include "Core/Log.h"
#include "Core/Project/Project.hpp"
#include "Utils/MathUtils.hpp"
#include "Utils/PlatformUtils.hpp"
#include "Utils/TEFileSystem.hpp"
#include <fstream>
#include <iomanip>

SpriteExportLayer::SpriteExportLayer(SpriteMode *mode) : Layer("SpriteExportLayer"), m_SpriteMode(mode) {}

void SpriteExportLayer::OnAttach() {}

void SpriteExportLayer::OnDetach() {}

void SpriteExportLayer::OnUpdate()
{
    if (!m_IsOpen || !m_SpriteMode || m_SpriteMode->m_PixelFrames.empty())
        return;

    float dt = TimeGUI::GetIO().DeltaTime;

    // Advance live playback timer for spritesheet animation preview
    if (m_AnimPlaying && m_AnimFPS > 0)
    {
        m_AnimTimer += dt;
        float frameDuration = 1.0f / (float)m_AnimFPS;
        if (m_AnimTimer >= frameDuration)
        {
            m_AnimTimer -= frameDuration;
            int frameCount = (int)m_SpriteMode->m_PixelFrames.size();
            if (frameCount > 0)
            {
                if (m_AnimLoop)
                {
                    m_AnimFrameIndex = (m_AnimFrameIndex + 1) % frameCount;
                }
                else
                {
                    if (m_AnimFrameIndex < frameCount - 1)
                        m_AnimFrameIndex++;
                    else
                        m_AnimPlaying = false;
                }
            }
        }
    }
}

void SpriteExportLayer::Open(SpriteMode *mode)
{
    m_SpriteMode = mode;
    m_IsOpen = true;

    if (!m_SpriteMode)
        return;

    if (Project::GetActive())
    {
        TEString assetDir = Project::GetAssetDirectory();
        if (!assetDir.empty())
            m_ExportPath = assetDir / "SavedSprites" / "Sprite.png";
        else
            m_ExportPath = "Resources/SavedSprites/Sprite.png";
    }
    else
    {
        m_ExportPath = "Resources/SavedSprites/Sprite.png";
    }

    int frameCount = (int)m_SpriteMode->m_PixelFrames.size();
    if (m_SpriteMode->m_ActiveFrameIndex >= 0 && m_SpriteMode->m_ActiveFrameIndex < frameCount)
        m_SingleFrameIndex = m_SpriteMode->m_ActiveFrameIndex;
    else
        m_SingleFrameIndex = 0;

    ResetCropToFull();

    if (frameCount > 0)
    {
        m_SheetCols = Clamp((int)std::ceil(Sqrt((float)frameCount)), 1, frameCount);
        m_SheetRows = (frameCount + m_SheetCols - 1) / m_SheetCols;
    }
    else
    {
        m_SheetCols = 1;
        m_SheetRows = 1;
    }

    m_SinglePreviewZoom = 1.0f;
    m_SinglePreviewPan = TEVector2(0.0f, 0.0f);
    m_SheetPreviewZoom = 1.0f;
    m_SheetPreviewPan = TEVector2(0.0f, 0.0f);
    m_AnimFrameIndex = 0;
    m_AnimTimer = 0.0f;
    m_AnimPlaying = true;
    m_ActiveDragHandle = -1;
}

void SpriteExportLayer::Close()
{
    m_IsOpen = false;
    m_ActiveDragHandle = -1;
}

void SpriteExportLayer::ResetCropToFull()
{
    if (m_SpriteMode)
    {
        m_CropX = 0;
        m_CropY = 0;
        m_CropW = Max(1, m_SpriteMode->m_PixelGridWidth);
        m_CropH = Max(1, m_SpriteMode->m_PixelGridHeight);
    }
    else
    {
        m_CropX = 0;
        m_CropY = 0;
        m_CropW = 32;
        m_CropH = 32;
    }
}

void SpriteExportLayer::OnTimeGUIRender()
{
    if (!m_IsOpen || !m_SpriteMode)
        return;

    TimeGUI::OpenPopup("Sprite Export Suite##ModalDialog");
    TEVector2 center = TimeGUI::GetMainViewport().Pos;
    TEVector2 size = TimeGUI::GetMainViewport().Size;
    float modalW = Min(1080.0f, size.x - 40.0f);
    float modalH = Min(740.0f, size.y - 40.0f);
    TimeGUI::SetNextWindowPos(TEVector2(center.x + (size.x - modalW) * 0.5f, center.y + (size.y - modalH) * 0.5f));
    TimeGUI::SetNextWindowSize(TEVector2(modalW, modalH));

    if (TimeGUI::BeginPopupModal("Sprite Export Suite##ModalDialog", &m_IsOpen, TimeGUIWindowFlags_NoResize))
    {
        TimeGUI::TextColored(TEVector4(0.35f, 0.75f, 1.0f, 1.0f), "ADVANCED SPRITE EXPORT SUITE");
        TimeGUI::SameLine(modalW - 170.0f);
        TimeGUI::TextDisabled("v2.0 Dual-Pipeline");
        TimeGUI::Separator();
        TimeGUI::Spacing();

        // Main Content Region inside Child to prevent any footer collision
        float contentHeight = modalH - 110.0f;
        if (TimeGUI::BeginChild("##SpriteExportContent", TEVector2(0.0f, contentHeight), false))
        {
            if (TimeGUI::BeginTabBar("##SpriteExportTabs", TimeGUITabBarFlags_None))
            {
                if (TimeGUI::BeginTabItem("Single Frame Export"))
                {
                    m_ActiveTab = SpriteExportTab::SingleFrame;
                    DrawSingleFramePipeline();
                    TimeGUI::EndTabItem();
                }

                if (TimeGUI::BeginTabItem("Animation Spritesheet"))
                {
                    m_ActiveTab = SpriteExportTab::Spritesheet;
                    DrawSpritesheetPipeline();
                    TimeGUI::EndTabItem();
                }

                TimeGUI::EndTabBar();
            }
            TimeGUI::EndChild();
        }

        // Dedicated Bottom Modal Action Footer
        TimeGUI::Separator();
        TimeGUI::Spacing();

        if (TimeGUI::Button("Cancel", TEVector2(110.0f, 28.0f)))
        {
            Close();
        }

        TimeGUI::SameLine(modalW - 175.0f);
        TimeGUI::PushStyleColor(TimeGUICol_Button, TEVector4(0.12f, 0.58f, 0.28f, 1.0f));
        TimeGUI::PushStyleColor(TimeGUICol_ButtonHovered, TEVector4(0.18f, 0.72f, 0.35f, 1.0f));
        TimeGUI::PushStyleColor(TimeGUICol_ButtonActive, TEVector4(0.08f, 0.45f, 0.20f, 1.0f));

        const char *exportBtnLabel = (m_ActiveTab == SpriteExportTab::SingleFrame) ? "EXPORT FRAME" : "EXPORT SHEET";
        if (TimeGUI::Button(exportBtnLabel, TEVector2(150.0f, 28.0f)))
        {
            if (m_ActiveTab == SpriteExportTab::SingleFrame)
                ExecuteSingleFrameExport();
            else
                ExecuteSpritesheetExport();

            Close();
        }

        TimeGUI::PopStyleColor(3);

        TimeGUI::EndPopup();
    }
}

void SpriteExportLayer::DrawSingleFramePipeline()
{
    if (!m_SpriteMode || m_SpriteMode->m_PixelFrames.empty())
    {
        TimeGUI::TextColored(TEVector4(1.0f, 0.4f, 0.4f, 1.0f), "No pixel frames available for export.");
        return;
    }

    int frameCount = (int)m_SpriteMode->m_PixelFrames.size();
    if (m_SingleFrameIndex < 0)
        m_SingleFrameIndex = 0;
    if (m_SingleFrameIndex >= frameCount)
        m_SingleFrameIndex = frameCount - 1;

    if (TimeGUI::BeginTable("##SingleFrameLayout", 2, TimeGUITableFlags_None))
    {
        TimeGUI::TableSetupColumn("PreviewCol", TimeGUITableColumnFlags_WidthFixed, 480.0f);
        TimeGUI::TableSetupColumn("SettingsCol", TimeGUITableColumnFlags_WidthStretch, 1.0f);

        // Column 1: Interactive Crop & Zoom Preview Box
        TimeGUI::TableNextColumn();
        TimeGUI::TextColored(TEVector4(0.9f, 0.9f, 0.3f, 1.0f), "Interactive Frame Preview");
        TimeGUI::Separator();

        // Frame Selector
        TimeGUI::Text("Active Frame: %d / %d", m_SingleFrameIndex + 1, frameCount);
        TimeGUI::SetNextItemWidth(340.0f);
        TimeGUI::SliderInt("##SingleFrameSlider", &m_SingleFrameIndex, 0, frameCount - 1, "Frame %d");

        TimeGUI::SameLine();
        if (TimeGUI::Button("<##PrevFrame", TEVector2(32.0f, 22.0f)))
        {
            if (m_SingleFrameIndex > 0)
                m_SingleFrameIndex--;
        }
        TimeGUI::SameLine();
        if (TimeGUI::Button(">##NextFrame", TEVector2(32.0f, 22.0f)))
        {
            if (m_SingleFrameIndex < frameCount - 1)
                m_SingleFrameIndex++;
        }

        // Preview Canvas Viewport
        TEVector2 previewBoxSize = TEVector2(460.0f, 400.0f);
        TEVector2 previewScreenPos = TimeGUI::GetCursorScreenPos();
        DrawSingleFramePreview(previewScreenPos, previewBoxSize);
        TimeGUI::Dummy(previewBoxSize);

        // Pan / Zoom Controls
        TimeGUI::Spacing();
        TimeGUI::Text("Zoom: %.2fx", m_SinglePreviewZoom);
        TimeGUI::SameLine();
        TimeGUI::SetNextItemWidth(160.0f);
        TimeGUI::SliderFloat("##SingleZoomSlider", &m_SinglePreviewZoom, 0.2f, 8.0f, "%.2fx");
        TimeGUI::SameLine();
        if (TimeGUI::Button("Reset View##SingleReset", TEVector2(85.0f, 22.0f)))
        {
            m_SinglePreviewZoom = 1.0f;
            m_SinglePreviewPan = TEVector2(0.0f, 0.0f);
        }
        TimeGUI::TextDisabled("(Right/Middle Drag: Pan | Wheel: Zoom | Left: Adjust Crop)");

        // Column 2: Export Configuration & Crop Controls
        TimeGUI::TableNextColumn();
        TimeGUI::TextColored(TEVector4(0.4f, 0.8f, 1.0f, 1.0f), "Single Frame Configuration");
        TimeGUI::Separator();

        // Destination Path
        TimeGUI::Text("Destination Path:");
        TimeGUI::SetNextItemWidth(360.0f);
        TimeGUI::InputText("##SingleExportPath", m_ExportPath);
        TimeGUI::SameLine();
        if (TimeGUI::Button("Browse...##SingleBrowse", TEVector2(75.0f, 22.0f)))
        {
            TEString path = PlatformUtils::SaveFile("PNG Image (*.png)\0*.png\0");
            if (!path.empty())
            {
                if (path.length() >= 4 && path.substr(path.length() - 4) != ".png")
                    path += ".png";
                m_ExportPath = path;
            }
        }

        TimeGUI::Spacing();
        TimeGUI::Checkbox("Preserve Clean Alpha Transparency", &m_ExportTransparent);
        if (!m_ExportTransparent)
        {
            TimeGUI::Text("Matte Background Color:");
            TimeGUI::ColorEdit4("##SingleBgColor", &m_BackgroundColor.x, TimeGUIColorEditFlags_NoAlpha);
        }

        TimeGUI::Spacing();
        TimeGUI::Text("Pixel Scaling Factor:");
        TimeGUI::RadioButton("1x (Native)", &m_ScaleMultiplier, 1);
        TimeGUI::SameLine();
        TimeGUI::RadioButton("2x", &m_ScaleMultiplier, 2);
        TimeGUI::SameLine();
        TimeGUI::RadioButton("4x", &m_ScaleMultiplier, 4);
        TimeGUI::SameLine();
        TimeGUI::RadioButton("8x", &m_ScaleMultiplier, 8);

        TimeGUI::Spacing();
        TimeGUI::Separator();
        TimeGUI::TextColored(TEVector4(1.0f, 0.8f, 0.4f, 1.0f), "Bounding Crop Region");

        TimeGUI::Checkbox("Enable Interactive Crop", &m_EnableCrop);

        int origW = m_SpriteMode->m_PixelGridWidth;
        int origH = m_SpriteMode->m_PixelGridHeight;

        if (m_EnableCrop)
        {
            TimeGUI::BeginChild("##CropControlsBox", TEVector2(0.0f, 140.0f), true);

            TimeGUI::SetNextItemWidth(100.0f);
            if (TimeGUI::DragInt("Crop X", &m_CropX, 1.0f, 0, origW - 1))
                m_CropX = Clamp(m_CropX, 0, origW - 1);

            TimeGUI::SameLine(180.0f);
            TimeGUI::SetNextItemWidth(100.0f);
            if (TimeGUI::DragInt("Crop Y", &m_CropY, 1.0f, 0, origH - 1))
                m_CropY = Clamp(m_CropY, 0, origH - 1);

            TimeGUI::SetNextItemWidth(100.0f);
            if (TimeGUI::DragInt("Width##CropW", &m_CropW, 1.0f, 1, origW - m_CropX))
                m_CropW = Clamp(m_CropW, 1, origW - m_CropX);

            TimeGUI::SameLine(180.0f);
            TimeGUI::SetNextItemWidth(100.0f);
            if (TimeGUI::DragInt("Height##CropH", &m_CropH, 1.0f, 1, origH - m_CropY))
                m_CropH = Clamp(m_CropH, 1, origH - m_CropY);

            TimeGUI::Spacing();
            if (TimeGUI::Button("Full Frame (Reset)", TEVector2(130.0f, 24.0f)))
            {
                ResetCropToFull();
            }
            TimeGUI::SameLine();
            if (TimeGUI::Button("Center Square Crop", TEVector2(140.0f, 24.0f)))
            {
                int side = Min(origW, origH);
                m_CropW = side;
                m_CropH = side;
                m_CropX = (origW - side) / 2;
                m_CropY = (origH - side) / 2;
            }

            TimeGUI::EndChild();
        }
        else
        {
            TimeGUI::TextDisabled("Whole Frame Mode (%d x %d px)", origW, origH);
        }

        // Final Export Summary
        int outW = (m_EnableCrop ? m_CropW : origW) * m_ScaleMultiplier;
        int outH = (m_EnableCrop ? m_CropH : origH) * m_ScaleMultiplier;
        TimeGUI::Spacing();
        TimeGUI::TextColored(TEVector4(0.2f, 0.9f, 0.4f, 1.0f), "Final Output Image: %d x %d px (%s)", outW, outH,
                             m_ExportTransparent ? "RGBA 32-bit" : "RGB 24-bit");

        TimeGUI::EndTable();
    }
}

void SpriteExportLayer::DrawSingleFramePreview(const TEVector2 &previewPos, const TEVector2 &previewSize)
{
    TimeGUI::TimeGUIDrawList dl = TimeGUI::GetWindowDrawList();

    // 1. Background Box & Clip Rect
    dl.AddRectFilled(previewPos, TEVector2(previewPos.x + previewSize.x, previewPos.y + previewSize.y),
                     IM_COL32(18, 18, 22, 255), 4.0f);
    dl.AddRect(previewPos, TEVector2(previewPos.x + previewSize.x, previewPos.y + previewSize.y),
               IM_COL32(45, 45, 55, 255), 4.0f);

    dl.PushClipRect(previewPos, TEVector2(previewPos.x + previewSize.x, previewPos.y + previewSize.y), true);

    int gridW = m_SpriteMode->m_PixelGridWidth;
    int gridH = m_SpriteMode->m_PixelGridHeight;
    float baseCellDim = Min((previewSize.x - 30.0f) / gridW, (previewSize.y - 30.0f) / gridH);
    float cellDim = baseCellDim * m_SinglePreviewZoom;

    TEVector2 canvasCenter = TEVector2(previewPos.x + previewSize.x * 0.5f + m_SinglePreviewPan.x,
                                       previewPos.y + previewSize.y * 0.5f + m_SinglePreviewPan.y);
    TEVector2 origin = TEVector2(canvasCenter.x - (gridW * cellDim) * 0.5f, canvasCenter.y - (gridH * cellDim) * 0.5f);

    // 2. Checkerboard pattern inside frame bounds (or matte background if transparency disabled)
    if (m_ExportTransparent)
    {
        RenderCheckerboard(dl, origin, TEVector2(gridW * cellDim, gridH * cellDim), Max(6.0f, cellDim));
    }
    else
    {
        dl.AddRectFilled(origin, TEVector2(origin.x + gridW * cellDim, origin.y + gridH * cellDim),
                         TimeGUI::ColorConvertFloat4ToU32(m_BackgroundColor));
    }

    // 3. Render composited frame pixels
    if (m_SingleFrameIndex >= 0 && m_SingleFrameIndex < (int)m_SpriteMode->m_PixelFrames.size())
    {
        const auto &frame = m_SpriteMode->m_PixelFrames[m_SingleFrameIndex];
        for (const auto &layer : frame.Layers)
        {
            if (!layer.Visible || layer.Pixels.size() < (size_t)(gridW * gridH))
                continue;

            for (int y = 0; y < gridH; y++)
            {
                for (int x = 0; x < gridW; x++)
                {
                    TEVector4 col = layer.Pixels[y * gridW + x];
                    if (col.w > 0.001f)
                    {
                        col.w *= layer.Opacity;
                        TEVector2 p1 = TEVector2(origin.x + x * cellDim, origin.y + y * cellDim);
                        TEVector2 p2 = TEVector2(origin.x + (x + 1) * cellDim, origin.y + (y + 1) * cellDim);
                        dl.AddRectFilled(p1, p2, TimeGUI::ColorConvertFloat4ToU32(col));
                    }
                }
            }
        }
    }

    // 4. Outer Sprite Frame Border
    dl.AddRect(origin, TEVector2(origin.x + gridW * cellDim, origin.y + gridH * cellDim), IM_COL32(90, 90, 110, 255),
               0.0f, 0, 1.5f);

    // 5. Interactive Crop Region Overlay
    if (m_EnableCrop)
    {
        // Clamp crop rect to frame dimensions
        m_CropX = Clamp(m_CropX, 0, gridW - 1);
        m_CropY = Clamp(m_CropY, 0, gridH - 1);
        m_CropW = Clamp(m_CropW, 1, gridW - m_CropX);
        m_CropH = Clamp(m_CropH, 1, gridH - m_CropY);

        TEVector2 cropP1 = TEVector2(origin.x + m_CropX * cellDim, origin.y + m_CropY * cellDim);
        TEVector2 cropP2 = TEVector2(cropP1.x + m_CropW * cellDim, cropP1.y + m_CropH * cellDim);

        // Dim outer areas
        TEVector2 frameP1 = origin;
        TEVector2 frameP2 = TEVector2(origin.x + gridW * cellDim, origin.y + gridH * cellDim);

        // Top band
        if (cropP1.y > frameP1.y)
            dl.AddRectFilled(frameP1, TEVector2(frameP2.x, cropP1.y), IM_COL32(0, 0, 0, 150));
        // Bottom band
        if (cropP2.y < frameP2.y)
            dl.AddRectFilled(TEVector2(frameP1.x, cropP2.y), frameP2, IM_COL32(0, 0, 0, 150));
        // Left band
        if (cropP1.x > frameP1.x)
            dl.AddRectFilled(TEVector2(frameP1.x, cropP1.y), TEVector2(cropP1.x, cropP2.y), IM_COL32(0, 0, 0, 150));
        // Right band
        if (cropP2.x < frameP2.x)
            dl.AddRectFilled(TEVector2(cropP2.x, cropP1.y), TEVector2(frameP2.x, cropP2.y), IM_COL32(0, 0, 0, 150));

        // Crop Bounding Box Outline
        dl.AddRect(cropP1, cropP2, IM_COL32(0, 220, 255, 255), 0.0f, 0, 2.0f);

        // Corner & Edge Handles
        float handleSize = 7.0f;
        auto drawHandle = [&](const TEVector2 &center)
        {
            dl.AddRectFilled(TEVector2(center.x - handleSize * 0.5f, center.y - handleSize * 0.5f),
                             TEVector2(center.x + handleSize * 0.5f, center.y + handleSize * 0.5f),
                             IM_COL32(255, 255, 255, 255), 1.0f);
            dl.AddRect(TEVector2(center.x - handleSize * 0.5f, center.y - handleSize * 0.5f),
                       TEVector2(center.x + handleSize * 0.5f, center.y + handleSize * 0.5f),
                       IM_COL32(0, 150, 220, 255), 1.0f, 0, 1.5f);
        };

        drawHandle(cropP1);                                            // TL (1)
        drawHandle(TEVector2(cropP2.x, cropP1.y));                     // TR (2)
        drawHandle(TEVector2(cropP1.x, cropP2.y));                     // BL (3)
        drawHandle(cropP2);                                            // BR (4)
        drawHandle(TEVector2(cropP1.x, (cropP1.y + cropP2.y) * 0.5f)); // L (5)
        drawHandle(TEVector2(cropP2.x, (cropP1.y + cropP2.y) * 0.5f)); // R (6)
        drawHandle(TEVector2((cropP1.x + cropP2.x) * 0.5f, cropP1.y)); // T (7)
        drawHandle(TEVector2((cropP1.x + cropP2.x) * 0.5f, cropP2.y)); // B (8)

        // Text Dimensions overlay
        TEString dimText = TEString::Format("%d x %d", m_CropW, m_CropH);
        dl.AddText(TEVector2(cropP1.x + 4.0f, cropP1.y + 4.0f), IM_COL32(0, 230, 255, 255), dimText);
    }

    dl.PopClipRect();

    // 6. Interactive Mouse Navigation & Draggable Crop Bounding Box
    TEVector2 mousePos = TimeGUI::GetMousePos();
    bool isHovered = mousePos.x >= previewPos.x && mousePos.x <= previewPos.x + previewSize.x &&
                     mousePos.y >= previewPos.y && mousePos.y <= previewPos.y + previewSize.y;

    if (isHovered)
    {
        // Pan with Right / Middle Click
        if (TimeGUI::IsMouseDragging(TimeGUIMouseButton_Right) || TimeGUI::IsMouseDragging(TimeGUIMouseButton_Middle))
        {
            m_SinglePreviewPan.x += TimeGUI::GetIO().MouseDelta.x;
            m_SinglePreviewPan.y += TimeGUI::GetIO().MouseDelta.y;
        }

        // Zoom with Mouse Wheel
        float wheel = TimeGUI::GetIO().MouseWheel;
        if (wheel != 0.0f)
        {
            m_SinglePreviewZoom = Clamp(m_SinglePreviewZoom + wheel * 0.15f, 0.2f, 10.0f);
        }

        // Draggable Crop Box Interaction
        if (m_EnableCrop)
        {
            TEVector2 cropP1 = TEVector2(origin.x + m_CropX * cellDim, origin.y + m_CropY * cellDim);
            TEVector2 cropP2 = TEVector2(cropP1.x + m_CropW * cellDim, cropP1.y + m_CropH * cellDim);
            float hitRadius = 9.0f;

            auto isNear = [&](const TEVector2 &pt)
            { return Abs(mousePos.x - pt.x) <= hitRadius && Abs(mousePos.y - pt.y) <= hitRadius; };

            int hoveredHandle = -1;
            if (isNear(cropP1))
                hoveredHandle = 1; // TL
            else if (isNear(TEVector2(cropP2.x, cropP1.y)))
                hoveredHandle = 2; // TR
            else if (isNear(TEVector2(cropP1.x, cropP2.y)))
                hoveredHandle = 3; // BL
            else if (isNear(cropP2))
                hoveredHandle = 4; // BR
            else if (Abs(mousePos.x - cropP1.x) <= hitRadius && mousePos.y >= cropP1.y && mousePos.y <= cropP2.y)
                hoveredHandle = 5; // L
            else if (Abs(mousePos.x - cropP2.x) <= hitRadius && mousePos.y >= cropP1.y && mousePos.y <= cropP2.y)
                hoveredHandle = 6; // R
            else if (Abs(mousePos.y - cropP1.y) <= hitRadius && mousePos.x >= cropP1.x && mousePos.x <= cropP2.x)
                hoveredHandle = 7; // T
            else if (Abs(mousePos.y - cropP2.y) <= hitRadius && mousePos.x >= cropP1.x && mousePos.x <= cropP2.x)
                hoveredHandle = 8; // B
            else if (mousePos.x > cropP1.x && mousePos.x < cropP2.x && mousePos.y > cropP1.y && mousePos.y < cropP2.y)
                hoveredHandle = 0; // Interior Move

            // Mouse Cursor Feedback
            if (hoveredHandle == 1 || hoveredHandle == 4)
                TimeGUI::SetMouseCursor(TimeGUIMouseCursor_ResizeNWSE);
            else if (hoveredHandle == 2 || hoveredHandle == 3)
                TimeGUI::SetMouseCursor(TimeGUIMouseCursor_ResizeNESW);
            else if (hoveredHandle == 5 || hoveredHandle == 6)
                TimeGUI::SetMouseCursor(TimeGUIMouseCursor_ResizeEW);
            else if (hoveredHandle == 7 || hoveredHandle == 8)
                TimeGUI::SetMouseCursor(TimeGUIMouseCursor_ResizeNS);
            else if (hoveredHandle == 0)
                TimeGUI::SetMouseCursor(TimeGUIMouseCursor_ResizeAll);

            // Handle Click Initiation
            if (TimeGUI::IsMouseClicked(TimeGUIMouseButton_Left) && hoveredHandle >= 0)
            {
                m_ActiveDragHandle = hoveredHandle;
                m_CropDragMouseStart = mousePos;
                m_CropDragInitialX = m_CropX;
                m_CropDragInitialY = m_CropY;
                m_CropDragInitialW = m_CropW;
                m_CropDragInitialH = m_CropH;
            }
        }
    }

    // Active Drag Processing
    if (m_ActiveDragHandle >= 0)
    {
        if (TimeGUI::IsMouseDown(TimeGUIMouseButton_Left))
        {
            int dx = (int)(mousePos.x - m_CropDragMouseStart.x >= 0
                               ? (mousePos.x - m_CropDragMouseStart.x) / cellDim + 0.5f
                               : (mousePos.x - m_CropDragMouseStart.x) / cellDim - 0.5f);
            int dy = (int)(mousePos.y - m_CropDragMouseStart.y >= 0
                               ? (mousePos.y - m_CropDragMouseStart.y) / cellDim + 0.5f
                               : (mousePos.y - m_CropDragMouseStart.y) / cellDim - 0.5f);

            if (m_ActiveDragHandle == 0) // Center Move
            {
                m_CropX = Clamp(m_CropDragInitialX + dx, 0, gridW - m_CropW);
                m_CropY = Clamp(m_CropDragInitialY + dy, 0, gridH - m_CropH);
            }
            else if (m_ActiveDragHandle == 1) // TL
            {
                int newX = Clamp(m_CropDragInitialX + dx, 0, m_CropDragInitialX + m_CropDragInitialW - 1);
                int newY = Clamp(m_CropDragInitialY + dy, 0, m_CropDragInitialY + m_CropDragInitialH - 1);
                m_CropW = (m_CropDragInitialX + m_CropDragInitialW) - newX;
                m_CropH = (m_CropDragInitialY + m_CropDragInitialH) - newY;
                m_CropX = newX;
                m_CropY = newY;
            }
            else if (m_ActiveDragHandle == 2) // TR
            {
                int newY = Clamp(m_CropDragInitialY + dy, 0, m_CropDragInitialY + m_CropDragInitialH - 1);
                m_CropW = Clamp(m_CropDragInitialW + dx, 1, gridW - m_CropX);
                m_CropH = (m_CropDragInitialY + m_CropDragInitialH) - newY;
                m_CropY = newY;
            }
            else if (m_ActiveDragHandle == 3) // BL
            {
                int newX = Clamp(m_CropDragInitialX + dx, 0, m_CropDragInitialX + m_CropDragInitialW - 1);
                m_CropW = (m_CropDragInitialX + m_CropDragInitialW) - newX;
                m_CropH = Clamp(m_CropDragInitialH + dy, 1, gridH - m_CropY);
                m_CropX = newX;
            }
            else if (m_ActiveDragHandle == 4) // BR
            {
                m_CropW = Clamp(m_CropDragInitialW + dx, 1, gridW - m_CropX);
                m_CropH = Clamp(m_CropDragInitialH + dy, 1, gridH - m_CropY);
            }
            else if (m_ActiveDragHandle == 5) // L
            {
                int newX = Clamp(m_CropDragInitialX + dx, 0, m_CropDragInitialX + m_CropDragInitialW - 1);
                m_CropW = (m_CropDragInitialX + m_CropDragInitialW) - newX;
                m_CropX = newX;
            }
            else if (m_ActiveDragHandle == 6) // R
            {
                m_CropW = Clamp(m_CropDragInitialW + dx, 1, gridW - m_CropX);
            }
            else if (m_ActiveDragHandle == 7) // T
            {
                int newY = Clamp(m_CropDragInitialY + dy, 0, m_CropDragInitialY + m_CropDragInitialH - 1);
                m_CropH = (m_CropDragInitialY + m_CropDragInitialH) - newY;
                m_CropY = newY;
            }
            else if (m_ActiveDragHandle == 8) // B
            {
                m_CropH = Clamp(m_CropDragInitialH + dy, 1, gridH - m_CropY);
            }
        }
        else
        {
            m_ActiveDragHandle = -1;
        }
    }
}

void SpriteExportLayer::DrawSpritesheetPipeline()
{
    if (!m_SpriteMode || m_SpriteMode->m_PixelFrames.empty())
    {
        TimeGUI::TextColored(TEVector4(1.0f, 0.4f, 0.4f, 1.0f), "No pixel frames available for spritesheet export.");
        return;
    }

    int frameCount = (int)m_SpriteMode->m_PixelFrames.size();
    int gridW = m_SpriteMode->m_PixelGridWidth;
    int gridH = m_SpriteMode->m_PixelGridHeight;

    if (TimeGUI::BeginTable("##SpritesheetLayout", 2, TimeGUITableFlags_None))
    {
        TimeGUI::TableSetupColumn("SheetPreviewCol", TimeGUITableColumnFlags_WidthFixed, 480.0f);
        TimeGUI::TableSetupColumn("SheetSettingsCol", TimeGUITableColumnFlags_WidthStretch, 1.0f);

        // Column 1: Live Playback & Spritesheet Preview
        TimeGUI::TableNextColumn();
        TimeGUI::TextColored(TEVector4(0.9f, 0.9f, 0.3f, 1.0f), "Live Preview");
        TimeGUI::SameLine(220.0f);
        TimeGUI::RadioButton("Animated##AnimTab", (int *)&m_ShowFullSheetPreview, 0);
        TimeGUI::SameLine();
        TimeGUI::RadioButton("Sheet Grid##GridTab", (int *)&m_ShowFullSheetPreview, 1);
        TimeGUI::Separator();

        // Animation Controls
        if (!m_ShowFullSheetPreview)
        {
            TimeGUI::Text("Playing Frame: %d / %d", m_AnimFrameIndex + 1, frameCount);
            TimeGUI::SameLine(200.0f);
            if (TimeGUI::Button(m_AnimPlaying ? "Pause##PlayPause" : "Play##PlayPause", TEVector2(60.0f, 22.0f)))
            {
                m_AnimPlaying = !m_AnimPlaying;
            }
            TimeGUI::SameLine();
            if (TimeGUI::Button("<##StepPrev", TEVector2(28.0f, 22.0f)))
            {
                m_AnimPlaying = false;
                m_AnimFrameIndex = (m_AnimFrameIndex - 1 + frameCount) % frameCount;
            }
            TimeGUI::SameLine();
            if (TimeGUI::Button(">##StepNext", TEVector2(28.0f, 22.0f)))
            {
                m_AnimPlaying = false;
                m_AnimFrameIndex = (m_AnimFrameIndex + 1) % frameCount;
            }
            TimeGUI::SameLine();
            TimeGUI::Checkbox("Loop", &m_AnimLoop);

            TimeGUI::SetNextItemWidth(180.0f);
            TimeGUI::SliderInt("Playback FPS", &m_AnimFPS, 1, 60, "%d fps");
        }
        else
        {
            TimeGUI::Text("Full Grid Layout Preview (Total Frames: %d)", frameCount);
            TimeGUI::TextDisabled("(Right/Middle Drag: Pan | Wheel: Zoom)");
        }

        // Preview Box Viewport
        TEVector2 previewBoxSize = TEVector2(460.0f, 380.0f);
        TEVector2 previewScreenPos = TimeGUI::GetCursorScreenPos();
        DrawSpritesheetPreview(previewScreenPos, previewBoxSize);
        TimeGUI::Dummy(previewBoxSize);

        // Zoom / Pan Footer for Sheet View
        if (m_ShowFullSheetPreview)
        {
            TimeGUI::Spacing();
            TimeGUI::Text("Zoom: %.2fx", m_SheetPreviewZoom);
            TimeGUI::SameLine();
            TimeGUI::SetNextItemWidth(140.0f);
            TimeGUI::SliderFloat("##SheetZoomSlider", &m_SheetPreviewZoom, 0.2f, 6.0f, "%.2fx");
            TimeGUI::SameLine();
            if (TimeGUI::Button("Reset View##SheetReset", TEVector2(85.0f, 22.0f)))
            {
                m_SheetPreviewZoom = 1.0f;
                m_SheetPreviewPan = TEVector2(0.0f, 0.0f);
            }
        }

        // Column 2: Spritesheet Grid Configuration & Metadata
        TimeGUI::TableNextColumn();
        TimeGUI::TextColored(TEVector4(0.4f, 0.8f, 1.0f, 1.0f), "Spritesheet Grid & Metadata");
        TimeGUI::Separator();

        // Destination Path
        TimeGUI::Text("Destination Texture PNG:");
        TimeGUI::SetNextItemWidth(360.0f);
        TimeGUI::InputText("##SheetExportPath", m_ExportPath);
        TimeGUI::SameLine();
        if (TimeGUI::Button("Browse...##SheetBrowse", TEVector2(75.0f, 22.0f)))
        {
            TEString path = PlatformUtils::SaveFile("PNG Image (*.png)\0*.png\0");
            if (!path.empty())
            {
                if (path.length() >= 4 && path.substr(path.length() - 4) != ".png")
                    path += ".png";
                m_ExportPath = path;
            }
        }

        TimeGUI::Spacing();
        TimeGUI::Checkbox("Preserve Clean Alpha Transparency##Sheet", &m_ExportTransparent);
        if (!m_ExportTransparent)
        {
            TimeGUI::Text("Matte Background Color:");
            TimeGUI::ColorEdit4("##SheetBgColor", &m_BackgroundColor.x, TimeGUIColorEditFlags_NoAlpha);
        }

        TimeGUI::Spacing();
        TimeGUI::Text("Pixel Scaling Multiplier:");
        TimeGUI::RadioButton("1x##SheetScale", &m_ScaleMultiplier, 1);
        TimeGUI::SameLine();
        TimeGUI::RadioButton("2x##SheetScale", &m_ScaleMultiplier, 2);
        TimeGUI::SameLine();
        TimeGUI::RadioButton("4x##SheetScale", &m_ScaleMultiplier, 4);
        TimeGUI::SameLine();
        TimeGUI::RadioButton("8x##SheetScale", &m_ScaleMultiplier, 8);

        TimeGUI::Spacing();
        TimeGUI::Separator();
        TimeGUI::TextColored(TEVector4(1.0f, 0.8f, 0.4f, 1.0f), "Grid Layout Parameters");

        TimeGUI::SetNextItemWidth(120.0f);
        if (TimeGUI::InputInt("Columns##SheetCols", &m_SheetCols))
        {
            if (m_SheetCols < 1)
                m_SheetCols = 1;
        }

        m_SheetRows = (frameCount + m_SheetCols - 1) / m_SheetCols;
        TimeGUI::SameLine(180.0f);
        TimeGUI::Text("Rows: %d", m_SheetRows);

        TimeGUI::SetNextItemWidth(120.0f);
        TimeGUI::DragInt("Padding (px)##SheetPad", &m_SheetPadding, 1.0f, 0, 64);
        TimeGUI::SameLine(180.0f);
        TimeGUI::SetNextItemWidth(120.0f);
        TimeGUI::DragInt("Spacing (px)##SheetSpace", &m_SheetSpacing, 1.0f, 0, 64);

        int cellW = gridW * m_ScaleMultiplier;
        int cellH = gridH * m_ScaleMultiplier;
        int totalSheetW = m_SheetPadding * 2 + m_SheetCols * cellW + (m_SheetCols - 1) * m_SheetSpacing;
        int totalSheetH = m_SheetPadding * 2 + m_SheetRows * cellH + (m_SheetRows - 1) * m_SheetSpacing;

        TimeGUI::Spacing();
        TimeGUI::TextColored(TEVector4(0.2f, 0.9f, 0.4f, 1.0f), "Resulting Sheet: %d x %d px (%d Cols x %d Rows)",
                             totalSheetW, totalSheetH, m_SheetCols, m_SheetRows);

        TimeGUI::Spacing();
        TimeGUI::Separator();
        TimeGUI::TextColored(TEVector4(1.0f, 0.8f, 0.4f, 1.0f), "Metadata Generation");

        TimeGUI::Checkbox("Export Companion Metadata File", &m_ExportMetadata);
        if (m_ExportMetadata)
        {
            TimeGUI::Text("Metadata Format:");
            int formatInt = (int)m_MetadataFormat;
            TimeGUI::RadioButton("JSON Atlas (.json)", &formatInt, (int)SpriteMetadataFormat::JSON);
            TimeGUI::SameLine();
            TimeGUI::RadioButton("Native (.tesheet)", &formatInt, (int)SpriteMetadataFormat::TESheet);
            TimeGUI::SameLine();
            TimeGUI::RadioButton("Both (.json & .tesheet)", &formatInt, (int)SpriteMetadataFormat::Both);
            m_MetadataFormat = (SpriteMetadataFormat)formatInt;
        }

        TimeGUI::EndTable();
    }
}

void SpriteExportLayer::DrawSpritesheetPreview(const TEVector2 &previewPos, const TEVector2 &previewSize)
{
    TimeGUI::TimeGUIDrawList dl = TimeGUI::GetWindowDrawList();

    // Background Container
    dl.AddRectFilled(previewPos, TEVector2(previewPos.x + previewSize.x, previewPos.y + previewSize.y),
                     IM_COL32(18, 18, 22, 255), 4.0f);
    dl.AddRect(previewPos, TEVector2(previewPos.x + previewSize.x, previewPos.y + previewSize.y),
               IM_COL32(45, 45, 55, 255), 4.0f);

    dl.PushClipRect(previewPos, TEVector2(previewPos.x + previewSize.x, previewPos.y + previewSize.y), true);

    int gridW = m_SpriteMode->m_PixelGridWidth;
    int gridH = m_SpriteMode->m_PixelGridHeight;
    int frameCount = (int)m_SpriteMode->m_PixelFrames.size();

    if (!m_ShowFullSheetPreview)
    {
        // Live Single-Frame Animation Preview
        float baseCellDim = Min((previewSize.x - 30.0f) / gridW, (previewSize.y - 30.0f) / gridH);
        TEVector2 canvasCenter = TEVector2(previewPos.x + previewSize.x * 0.5f, previewPos.y + previewSize.y * 0.5f);
        TEVector2 origin =
            TEVector2(canvasCenter.x - (gridW * baseCellDim) * 0.5f, canvasCenter.y - (gridH * baseCellDim) * 0.5f);

        RenderCheckerboard(dl, origin, TEVector2(gridW * baseCellDim, gridH * baseCellDim), Max(6.0f, baseCellDim));

        if (m_AnimFrameIndex >= 0 && m_AnimFrameIndex < frameCount)
        {
            const auto &frame = m_SpriteMode->m_PixelFrames[m_AnimFrameIndex];
            for (const auto &layer : frame.Layers)
            {
                if (!layer.Visible || layer.Pixels.size() < (size_t)(gridW * gridH))
                    continue;

                for (int y = 0; y < gridH; y++)
                {
                    for (int x = 0; x < gridW; x++)
                    {
                        TEVector4 col = layer.Pixels[y * gridW + x];
                        if (col.w > 0.001f)
                        {
                            col.w *= layer.Opacity;
                            TEVector2 p1 = TEVector2(origin.x + x * baseCellDim, origin.y + y * baseCellDim);
                            TEVector2 p2 =
                                TEVector2(origin.x + (x + 1) * baseCellDim, origin.y + (y + 1) * baseCellDim);
                            dl.AddRectFilled(p1, p2, TimeGUI::ColorConvertFloat4ToU32(col));
                        }
                    }
                }
            }
        }

        dl.AddRect(origin, TEVector2(origin.x + gridW * baseCellDim, origin.y + gridH * baseCellDim),
                   IM_COL32(90, 90, 110, 255), 0.0f, 0, 1.5f);
    }
    else
    {
        // Full Spritesheet Grid Preview
        int cellW = gridW;
        int cellH = gridH;
        int cols = Max(1, m_SheetCols);
        int rows = Max(1, m_SheetRows);

        float totalPreviewW = (float)(m_SheetPadding * 2 + cols * cellW + (cols - 1) * m_SheetSpacing);
        float totalPreviewH = (float)(m_SheetPadding * 2 + rows * cellH + (rows - 1) * m_SheetSpacing);

        float baseScale = Min((previewSize.x - 30.0f) / totalPreviewW, (previewSize.y - 30.0f) / totalPreviewH);
        float finalScale = baseScale * m_SheetPreviewZoom;

        TEVector2 canvasCenter = TEVector2(previewPos.x + previewSize.x * 0.5f + m_SheetPreviewPan.x,
                                           previewPos.y + previewSize.y * 0.5f + m_SheetPreviewPan.y);
        TEVector2 origin = TEVector2(canvasCenter.x - (totalPreviewW * finalScale) * 0.5f,
                                     canvasCenter.y - (totalPreviewH * finalScale) * 0.5f);

        // Checkerboard behind the entire sheet
        RenderCheckerboard(dl, origin, TEVector2(totalPreviewW * finalScale, totalPreviewH * finalScale),
                           Max(8.0f, 16.0f * finalScale));

        for (int f = 0; f < frameCount; f++)
        {
            int c = f % cols;
            int r = f / cols;

            float cellOriginX = origin.x + (m_SheetPadding + c * (cellW + m_SheetSpacing)) * finalScale;
            float cellOriginY = origin.y + (m_SheetPadding + r * (cellH + m_SheetSpacing)) * finalScale;
            float cellPixelDim = finalScale;

            const auto &frame = m_SpriteMode->m_PixelFrames[f];
            for (const auto &layer : frame.Layers)
            {
                if (!layer.Visible || layer.Pixels.size() < (size_t)(gridW * gridH))
                    continue;

                for (int y = 0; y < gridH; y++)
                {
                    for (int x = 0; x < gridW; x++)
                    {
                        TEVector4 col = layer.Pixels[y * gridW + x];
                        if (col.w > 0.001f)
                        {
                            col.w *= layer.Opacity;
                            TEVector2 p1 = TEVector2(cellOriginX + x * cellPixelDim, cellOriginY + y * cellPixelDim);
                            TEVector2 p2 =
                                TEVector2(cellOriginX + (x + 1) * cellPixelDim, cellOriginY + (y + 1) * cellPixelDim);
                            dl.AddRectFilled(p1, p2, TimeGUI::ColorConvertFloat4ToU32(col));
                        }
                    }
                }
            }

            // Cell boundary line
            dl.AddRect(TEVector2(cellOriginX, cellOriginY),
                       TEVector2(cellOriginX + cellW * finalScale, cellOriginY + cellH * finalScale),
                       IM_COL32(80, 80, 100, 150), 0.0f, 0, 1.0f);
        }

        // Sheet outer border
        dl.AddRect(origin, TEVector2(origin.x + totalPreviewW * finalScale, origin.y + totalPreviewH * finalScale),
                   IM_COL32(0, 200, 255, 255), 0.0f, 0, 2.0f);
    }

    dl.PopClipRect();

    // Mouse Navigation for Full Sheet Preview
    if (m_ShowFullSheetPreview)
    {
        TEVector2 mousePos = TimeGUI::GetMousePos();
        bool isHovered = mousePos.x >= previewPos.x && mousePos.x <= previewPos.x + previewSize.x &&
                         mousePos.y >= previewPos.y && mousePos.y <= previewPos.y + previewSize.y;

        if (isHovered)
        {
            if (TimeGUI::IsMouseDragging(TimeGUIMouseButton_Right) ||
                TimeGUI::IsMouseDragging(TimeGUIMouseButton_Middle))
            {
                m_SheetPreviewPan.x += TimeGUI::GetIO().MouseDelta.x;
                m_SheetPreviewPan.y += TimeGUI::GetIO().MouseDelta.y;
            }

            float wheel = TimeGUI::GetIO().MouseWheel;
            if (wheel != 0.0f)
            {
                m_SheetPreviewZoom = Clamp(m_SheetPreviewZoom + wheel * 0.15f, 0.2f, 8.0f);
            }
        }
    }
}

void SpriteExportLayer::RenderCheckerboard(TimeGUI::TimeGUIDrawList &dl, const TEVector2 &pos, const TEVector2 &size,
                                           float checkSize)
{
    if (checkSize <= 1.0f)
        checkSize = 8.0f;

    for (float y = 0; y < size.y; y += checkSize)
    {
        for (float x = 0; x < size.x; x += checkSize)
        {
            int ix = (int)(x / checkSize);
            int iy = (int)(y / checkSize);
            unsigned int col = ((ix + iy) % 2 == 0) ? IM_COL32(28, 28, 34, 255) : IM_COL32(38, 38, 46, 255);
            dl.AddRectFilled(TEVector2(pos.x + x, pos.y + y),
                             TEVector2(pos.x + Min(x + checkSize, size.x), pos.y + Min(y + checkSize, size.y)), col);
        }
    }
}

void SpriteExportLayer::CompositeFramePixels(int frameIndex, int width, int height,
                                             TEArray<unsigned char> &outPixels) const
{
    outPixels.Resize(width * height * 4, 0);

    if (!m_SpriteMode || frameIndex < 0 || frameIndex >= (int)m_SpriteMode->m_PixelFrames.size())
        return;

    const auto &frame = m_SpriteMode->m_PixelFrames[frameIndex];

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            float accR = 0.0f, accG = 0.0f, accB = 0.0f, accA = 0.0f;

            for (const auto &layer : frame.Layers)
            {
                if (!layer.Visible || layer.Pixels.size() < (size_t)(width * height))
                    continue;

                TEVector4 col = layer.Pixels[y * width + x];
                float srcA = col.w * layer.Opacity;
                if (srcA <= 0.0001f)
                    continue;

                float srcR = col.x;
                float srcG = col.y;
                float srcB = col.z;

                // Exact Porter-Duff Over compositing for clean straight alpha
                float outA = srcA + accA * (1.0f - srcA);
                if (outA > 0.0f)
                {
                    accR = (srcR * srcA + accR * accA * (1.0f - srcA)) / outA;
                    accG = (srcG * srcA + accG * accA * (1.0f - srcA)) / outA;
                    accB = (srcB * srcA + accB * accA * (1.0f - srcA)) / outA;
                    accA = outA;
                }
            }

            int idx = (y * width + x) * 4;

            if (!m_ExportTransparent)
            {
                // Matte compositing over chosen background color
                float bgR = m_BackgroundColor.x;
                float bgG = m_BackgroundColor.y;
                float bgB = m_BackgroundColor.z;

                float finalR = accR * accA + bgR * (1.0f - accA);
                float finalG = accG * accA + bgG * (1.0f - accA);
                float finalB = accB * accA + bgB * (1.0f - accA);

                outPixels[idx + 0] = (unsigned char)Clamp((int)(finalR * 255.0f + 0.5f), 0, 255);
                outPixels[idx + 1] = (unsigned char)Clamp((int)(finalG * 255.0f + 0.5f), 0, 255);
                outPixels[idx + 2] = (unsigned char)Clamp((int)(finalB * 255.0f + 0.5f), 0, 255);
                outPixels[idx + 3] = 255;
            }
            else
            {
                // 100% clean alpha channel preservation
                outPixels[idx + 0] = (unsigned char)Clamp((int)(accR * 255.0f + 0.5f), 0, 255);
                outPixels[idx + 1] = (unsigned char)Clamp((int)(accG * 255.0f + 0.5f), 0, 255);
                outPixels[idx + 2] = (unsigned char)Clamp((int)(accB * 255.0f + 0.5f), 0, 255);
                outPixels[idx + 3] = (unsigned char)Clamp((int)(accA * 255.0f + 0.5f), 0, 255);
            }
        }
    }
}

void SpriteExportLayer::ExecuteSingleFrameExport()
{
    if (!m_SpriteMode || m_SpriteMode->m_PixelFrames.empty())
        return;

    int origW = m_SpriteMode->m_PixelGridWidth;
    int origH = m_SpriteMode->m_PixelGridHeight;

    TEArray<unsigned char> fullFramePixels;
    CompositeFramePixels(m_SingleFrameIndex, origW, origH, fullFramePixels);

    int cropX = 0, cropY = 0, cropW = origW, cropH = origH;
    if (m_EnableCrop)
    {
        cropX = Clamp(m_CropX, 0, origW - 1);
        cropY = Clamp(m_CropY, 0, origH - 1);
        cropW = Clamp(m_CropW, 1, origW - cropX);
        cropH = Clamp(m_CropH, 1, origH - cropY);
    }

    int scale = Max(1, m_ScaleMultiplier);
    int finalW = cropW * scale;
    int finalH = cropH * scale;

    TEArray<unsigned char> exportBuffer;
    exportBuffer.Resize(finalW * finalH * 4, 0);

    for (int y = 0; y < cropH; y++)
    {
        for (int x = 0; x < cropW; x++)
        {
            int srcIdx = ((cropY + y) * origW + (cropX + x)) * 4;
            unsigned char r = fullFramePixels[srcIdx + 0];
            unsigned char g = fullFramePixels[srcIdx + 1];
            unsigned char b = fullFramePixels[srcIdx + 2];
            unsigned char a = fullFramePixels[srcIdx + 3];

            for (int sy = 0; sy < scale; sy++)
            {
                for (int sx = 0; sx < scale; sx++)
                {
                    int dstIdx = ((y * scale + sy) * finalW + (x * scale + sx)) * 4;
                    exportBuffer[dstIdx + 0] = r;
                    exportBuffer[dstIdx + 1] = g;
                    exportBuffer[dstIdx + 2] = b;
                    exportBuffer[dstIdx + 3] = a;
                }
            }
        }
    }

    TEString finalPath = m_ExportPath;
    if (!finalPath.IsAbsolute())
    {
        if (Project::GetActive())
            finalPath = Project::GetProjectDirectory() / finalPath;
    }

    TEString exportDir = finalPath.GetParentPath();
    if (!exportDir.IsEmpty() && !TEFileSystem::Exists(exportDir))
        TEFileSystem::CreateDirectories(exportDir);

    AssetManager::ExportImagePNG(finalPath, finalW, finalH, 4, exportBuffer.GetData());
    TE_CORE_INFO("[SpriteExportLayer] Single Frame exported successfully ({0}x{1} px, Scale: {2}x) to: {3}", finalW,
                 finalH, scale, finalPath);
}

void SpriteExportLayer::ExecuteSpritesheetExport()
{
    if (!m_SpriteMode || m_SpriteMode->m_PixelFrames.empty())
        return;

    int frameCount = (int)m_SpriteMode->m_PixelFrames.size();
    int gridW = m_SpriteMode->m_PixelGridWidth;
    int gridH = m_SpriteMode->m_PixelGridHeight;
    int scale = Max(1, m_ScaleMultiplier);

    int cellW = gridW * scale;
    int cellH = gridH * scale;
    int cols = Max(1, m_SheetCols);
    int rows = Max(1, (frameCount + cols - 1) / cols);

    int sheetW = m_SheetPadding * 2 + cols * cellW + (cols - 1) * m_SheetSpacing;
    int sheetH = m_SheetPadding * 2 + rows * cellH + (rows - 1) * m_SheetSpacing;

    TEArray<unsigned char> sheetBytes;
    sheetBytes.Resize(sheetW * sheetH * 4, 0);

    for (int f = 0; f < frameCount; f++)
    {
        TEArray<unsigned char> framePixels;
        CompositeFramePixels(f, gridW, gridH, framePixels);

        int c = f % cols;
        int r = f / cols;
        int cellOriginX = m_SheetPadding + c * (cellW + m_SheetSpacing);
        int cellOriginY = m_SheetPadding + r * (cellH + m_SheetSpacing);

        for (int y = 0; y < gridH; y++)
        {
            for (int x = 0; x < gridW; x++)
            {
                int srcIdx = (y * gridW + x) * 4;
                unsigned char pr = framePixels[srcIdx + 0];
                unsigned char pg = framePixels[srcIdx + 1];
                unsigned char pb = framePixels[srcIdx + 2];
                unsigned char pa = framePixels[srcIdx + 3];

                for (int sy = 0; sy < scale; sy++)
                {
                    for (int sx = 0; sx < scale; sx++)
                    {
                        int dstX = cellOriginX + (x * scale + sx);
                        int dstY = cellOriginY + (y * scale + sy);
                        int dstIdx = (dstY * sheetW + dstX) * 4;

                        sheetBytes[dstIdx + 0] = pr;
                        sheetBytes[dstIdx + 1] = pg;
                        sheetBytes[dstIdx + 2] = pb;
                        sheetBytes[dstIdx + 3] = pa;
                    }
                }
            }
        }
    }

    TEString finalPath = m_ExportPath;
    if (!finalPath.IsAbsolute())
    {
        if (Project::GetActive())
            finalPath = Project::GetProjectDirectory() / finalPath;
    }

    TEString exportDir = finalPath.GetParentPath();
    if (!exportDir.IsEmpty() && !TEFileSystem::Exists(exportDir))
        TEFileSystem::CreateDirectories(exportDir);

    AssetManager::ExportImagePNG(finalPath, sheetW, sheetH, 4, sheetBytes.GetData());
    TE_CORE_INFO("[SpriteExportLayer] Animation Spritesheet exported successfully ({0}x{1} px, {2} frames) to: {3}",
                 sheetW, sheetH, frameCount, finalPath);

    // Optional Metadata Export
    if (m_ExportMetadata)
    {
        TEString basePath = finalPath;
        if (basePath.EndsWith(".png") || basePath.EndsWith(".PNG"))
            basePath = basePath.Left(basePath.Length() - 4);

        if (m_MetadataFormat == SpriteMetadataFormat::JSON || m_MetadataFormat == SpriteMetadataFormat::Both)
        {
            ExportMetadataJson(basePath + ".json", sheetW, sheetH, cellW, cellH, cols, rows, frameCount);
        }

        if (m_MetadataFormat == SpriteMetadataFormat::TESheet || m_MetadataFormat == SpriteMetadataFormat::Both)
        {
            ExportMetadataTESheet(basePath + ".tesheet", finalPath, sheetW, sheetH, cellW, cellH, cols, rows,
                                  frameCount);
        }
    }
}

void SpriteExportLayer::ExportMetadataJson(const TEString &jsonPath, int sheetW, int sheetH, int cellW, int cellH,
                                           int cols, int rows, int frameCount)
{
    std::ofstream out(jsonPath.c_str());
    if (!out.is_open())
    {
        TE_CORE_ERROR("[SpriteExportLayer] Failed to open metadata JSON file for writing: {0}", jsonPath);
        return;
    }

    TEString textureFilename = m_ExportPath.GetFilename();

    out << "{\n";
    out << "  \"meta\": {\n";
    out << "    \"app\": \"TimeEngine Sprite Editor\",\n";
    out << "    \"version\": \"2.0\",\n";
    out << "    \"image\": \"" << textureFilename.c_str() << "\",\n";
    out << "    \"format\": \"RGBA8888\",\n";
    out << "    \"size\": {\"w\": " << sheetW << ", \"h\": " << sheetH << "},\n";
    out << "    \"scale\": " << m_ScaleMultiplier << ",\n";
    out << "    \"frames_count\": " << frameCount << ",\n";
    out << "    \"columns\": " << cols << ",\n";
    out << "    \"rows\": " << rows << ",\n";
    out << "    \"padding\": " << m_SheetPadding << ",\n";
    out << "    \"spacing\": " << m_SheetSpacing << ",\n";
    out << "    \"cell_size\": {\"w\": " << cellW << ", \"h\": " << cellH << "},\n";
    out << "    \"fps\": " << m_AnimFPS << "\n";
    out << "  },\n";
    out << "  \"frames\": [\n";

    for (int f = 0; f < frameCount; f++)
    {
        int c = f % cols;
        int r = f / cols;
        int cellX = m_SheetPadding + c * (cellW + m_SheetSpacing);
        int cellY = m_SheetPadding + r * (cellH + m_SheetSpacing);
        int durationMs = (m_AnimFPS > 0) ? (int)(1000.0f / (float)m_AnimFPS) : 83;

        out << "    {\n";
        out << "      \"filename\": \"frame_" << f << "\",\n";
        out << "      \"frame\": {\"x\": " << cellX << ", \"y\": " << cellY << ", \"w\": " << cellW
            << ", \"h\": " << cellH << "},\n";
        out << "      \"rotated\": false,\n";
        out << "      \"trimmed\": false,\n";
        out << "      \"spriteSourceSize\": {\"x\": 0, \"y\": 0, \"w\": " << cellW << ", \"h\": " << cellH << "},\n";
        out << "      \"sourceSize\": {\"w\": " << cellW << ", \"h\": " << cellH << "},\n";
        out << "      \"duration\": " << durationMs << "\n";
        out << "    }" << (f + 1 < frameCount ? ",\n" : "\n");
    }

    out << "  ]\n";
    out << "}\n";
    out.close();

    TE_CORE_INFO("[SpriteExportLayer] Metadata JSON exported successfully to: {0}", jsonPath);
}

void SpriteExportLayer::ExportMetadataTESheet(const TEString &tesheetPath, const TEString &texturePath, int sheetW,
                                              int sheetH, int cellW, int cellH, int cols, int rows, int frameCount)
{
    std::ofstream out(tesheetPath.c_str());
    if (!out.is_open())
    {
        TE_CORE_ERROR("[SpriteExportLayer] Failed to open .tesheet file for writing: {0}", tesheetPath);
        return;
    }

    TEString sheetName = tesheetPath.GetStem();

    out << "SpriteSheet: " << sheetName.c_str() << "\n";
    out << "TexturePath: " << texturePath.c_str() << "\n";
    out << "CellWidth: " << cellW << "\n";
    out << "CellHeight: " << cellH << "\n";
    out << "PaddingX: " << m_SheetPadding << "\n";
    out << "PaddingY: " << m_SheetPadding << "\n";
    out << "OffsetX: " << m_SheetPadding << "\n";
    out << "OffsetY: " << m_SheetPadding << "\n";

    out << "SubFrameCount: " << frameCount << "\n";
    for (int f = 0; f < frameCount; f++)
    {
        int c = f % cols;
        int r = f / cols;
        int cellX = m_SheetPadding + c * (cellW + m_SheetSpacing);
        int cellY = m_SheetPadding + r * (cellH + m_SheetSpacing);

        float u0 = (sheetW > 0) ? ((float)cellX / (float)sheetW) : 0.0f;
        float v0 = (sheetH > 0) ? ((float)cellY / (float)sheetH) : 0.0f;
        float u1 = (sheetW > 0) ? ((float)(cellX + cellW) / (float)sheetW) : 1.0f;
        float v1 = (sheetH > 0) ? ((float)(cellY + cellH) / (float)sheetH) : 1.0f;

        out << "SubFrame: Frame_" << f << "," << f << "," << cellX << "," << cellY << "," << cellW << "," << cellH
            << "," << u0 << "," << v0 << "," << u1 << "," << v1 << "\n";
    }

    out << "AnimCount: 1\n";
    out << "Anim: Default," << (float)m_AnimFPS << "," << (m_AnimLoop ? "1" : "0") << ",";
    for (int f = 0; f < frameCount; f++)
    {
        out << f << (f + 1 < frameCount ? ";" : "");
    }
    out << "\n";

    out.close();
    TE_CORE_INFO("[SpriteExportLayer] Native .tesheet metadata exported successfully to: {0}", tesheetPath);
}
