#pragma once

#include "../SpriteEditorTypes.hpp"
#include "Layers/Layer.hpp"
#include "Utils/TimeGUI.hpp"

class SpriteMode;

enum class SpriteExportTab
{
    SingleFrame = 0,
    Spritesheet = 1
};

enum class SpriteMetadataFormat
{
    JSON = 0,
    TESheet = 1,
    Both = 2
};

class SpriteExportLayer : public Layer
{
public:
    SpriteExportLayer(SpriteMode *mode = nullptr);
    virtual ~SpriteExportLayer() = default;

    virtual void OnAttach() override;
    virtual void OnDetach() override;
    virtual void OnUpdate() override;
    virtual void OnTimeGUIRender() override;

    void Open(SpriteMode *mode);
    void Close();
    bool IsOpen() const { return m_IsOpen; }

private:
    // UI Layout Methods
    void DrawSingleFramePipeline();
    void DrawSpritesheetPipeline();
    void DrawSingleFramePreview(const TEVector2 &previewPos, const TEVector2 &previewSize);
    void DrawSpritesheetPreview(const TEVector2 &previewPos, const TEVector2 &previewSize);
    void RenderCheckerboard(TimeGUI::TimeGUIDrawList &dl, const TEVector2 &pos, const TEVector2 &size, float checkSize);

    // Compositing & Exporting
    void CompositeFramePixels(int frameIndex, int width, int height, TEArray<unsigned char> &outPixels) const;
    void ExecuteSingleFrameExport();
    void ExecuteSpritesheetExport();

    void ExportMetadataJson(const TEString &jsonPath, int sheetW, int sheetH, int cellW, int cellH, int cols, int rows,
                            int frameCount);
    void ExportMetadataTESheet(const TEString &tesheetPath, const TEString &texturePath, int sheetW, int sheetH,
                               int cellW, int cellH, int cols, int rows, int frameCount);

    void ResetCropToFull();

private:
    SpriteMode *m_SpriteMode = nullptr;
    bool m_IsOpen = false;
    SpriteExportTab m_ActiveTab = SpriteExportTab::SingleFrame;

    // Destination Path & Common Settings
    TEString m_ExportPath = "Sandbox/SavedSprites/Sprite.png";
    bool m_ExportTransparent = true;
    TEVector4 m_BackgroundColor = TEVector4(0.0f, 0.0f, 0.0f, 1.0f);
    int m_ScaleMultiplier = 1;

    // Single Frame Pipeline State
    int m_SingleFrameIndex = 0;
    bool m_EnableCrop = false;
    int m_CropX = 0;
    int m_CropY = 0;
    int m_CropW = 32;
    int m_CropH = 32;
    float m_SinglePreviewZoom = 1.0f;
    TEVector2 m_SinglePreviewPan = TEVector2(0.0f, 0.0f);
    int m_ActiveDragHandle = -1; // -1: none, 0: center/move, 1: TL, 2: TR, 3: BL, 4: BR, 5: L, 6: R, 7: T, 8: B
    TEVector2 m_CropDragMouseStart = TEVector2(0.0f, 0.0f);
    int m_CropDragInitialX = 0;
    int m_CropDragInitialY = 0;
    int m_CropDragInitialW = 0;
    int m_CropDragInitialH = 0;

    // Spritesheet Pipeline State
    int m_SheetCols = 4;
    int m_SheetRows = 1;
    int m_SheetPadding = 0;
    int m_SheetSpacing = 0;
    bool m_ExportMetadata = true;
    SpriteMetadataFormat m_MetadataFormat = SpriteMetadataFormat::Both;

    // Spritesheet Live Playback & Preview State
    bool m_AnimPlaying = true;
    int m_AnimFPS = 12;
    float m_AnimTimer = 0.0f;
    int m_AnimFrameIndex = 0;
    bool m_AnimLoop = true;
    bool m_ShowFullSheetPreview = false;
    float m_SheetPreviewZoom = 1.0f;
    TEVector2 m_SheetPreviewPan = TEVector2(0.0f, 0.0f);
};
