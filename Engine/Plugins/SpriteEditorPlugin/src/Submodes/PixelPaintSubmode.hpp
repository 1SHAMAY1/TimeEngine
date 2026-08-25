#pragma once

#include "ISubmode.hpp"
#include "../SpriteEditorTypes.hpp"
#include <vector>


enum class PixelToolType
{
    Pencil = 0,
    MirrorPen = 1,
    PaintBucket = 2,
    ColorReplaceBucket = 3,
    Eraser = 4,
    Line = 5,
    Rectangle = 6,
    Circle = 7,
    DitheringPen = 8,
    LightenDarken = 9,
    ColorPicker = 10
};

class PixelPaintSubmode : public ISubmode
{
public:
    PixelPaintSubmode();
    virtual ~PixelPaintSubmode() = default;

    virtual void OnEnter(SpriteMode *mode) override;
    virtual void OnUpdate(float dt, SpriteMode *mode) override;
    virtual void OnTimeGUIRender(SpriteEditorLayer *layer, SpriteMode *mode) override;
    virtual void OnExit(SpriteMode *mode) override;
    virtual bool OnShortcut(const TEString &shortcutId, SpriteMode *mode) override;

    virtual TEString GetName() const override { return "Pixel Paint"; }
    virtual TEString GetIcon() const override { return "P"; }

private:
    // Piskel-Style Panels
    void DrawToolPalette(SpriteMode *mode);
    void DrawAnimationFrameStrip(SpriteMode *mode);
    void DrawCanvasViewport(SpriteMode *mode);
    void DrawLiveAnimatedPreview(SpriteMode *mode);
    void DrawLayersPanel(SpriteMode *mode);
    void DrawTransformPanel(SpriteMode *mode);
    void DrawPalettesPanel(SpriteMode *mode);
    void DrawResizeModal(SpriteMode *mode);

    // Pixel Manipulation Algorithms
    void FloodFill(TEArray<TEVector4> &pixels, int width, int height, int startX, int startY, const TEVector4 &targetCol, const TEVector4 &fillCol);
    void ReplaceColor(TEArray<TEVector4> &pixels, int width, int height, const TEVector4 &targetCol, const TEVector4 &fillCol);
    void DrawBrushPixel(TEArray<TEVector4> &pixels, int width, int height, int x, int y, const TEVector4 &col, int size);
    void DrawDitherPixel(TEArray<TEVector4> &pixels, int width, int height, int x, int y, const TEVector4 &col, int size);
    void DrawBresenhamLine(TEArray<TEVector4> &pixels, int width, int height, int x0, int y0, int x1, int y1, const TEVector4 &col, int size);

    // Layer Composite Helper
    TEArray<TEVector4> GetCompositeFramePixels(SpriteMode *mode, int frameIndex);

private:
    PixelToolType m_ActiveTool = PixelToolType::Pencil;
    int m_BrushSize = 1; // 1px, 2px, 3px, 4px
    bool m_ShowGrid = true;
    bool m_EnableOnionSkin = true;

    // Color Swatches (Primary Left-Click, Secondary Right-Click)
    TEVector4 m_SecondaryColor = TEVector4(0.0f, 0.0f, 0.0f, 1.0f);

    // Live Animation Preview
    float m_AnimationTimer = 0.0f;
    int m_PreviewFrameIndex = 0;
    int m_PreviewFPS = 8; // 1 to 30 FPS
    bool m_IsPlayingPreview = true;

    // Canvas Navigation
    TEVector2 m_CanvasPan = TEVector2(0.0f, 0.0f);
    float m_CanvasZoom = 1.0f;
    bool m_IsPainting = false;
    TEVector2 m_ShapeStartCoord = TEVector2(-1, -1);

    // Resize Dialog State
    bool m_ShowResizeDialog = false;
    int m_NewResizeWidth = 32;
    int m_NewResizeHeight = 32;
    int m_AnchorGridIndex = 4; // 0..8 (4 is Center)
    bool m_MaintainAspectRatio = true;
    bool m_ResizeCanvasOnly = true; // true: Crop/expand canvas, false: scale content
};

