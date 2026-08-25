#pragma once

#include "SpriteEditorTypes.hpp"
#include "Utils/TimeGUI.hpp"


class SpriteMode;

class SpriteExportDialog
{
public:
    SpriteExportDialog() = default;
    ~SpriteExportDialog() = default;

    void Open(SpriteMode *mode);
    void Close();
    bool IsOpen() const { return m_IsOpen; }

    void OnTimeGUIRender(SpriteMode *mode);

private:
    void RenderPreview(SpriteMode *mode);
    void ExecuteExport(SpriteMode *mode);

private:
    bool m_IsOpen = false;
    bool m_ExportAsSpritesheet = false;
    bool m_ExportTransparent = true;
    bool m_PreviewDirty = true;

    TEString m_ExportPath = "TimeEditor/SavedSprites/Sprite.png";
    int m_OutputWidth = 128;
    int m_OutputHeight = 128;
    int m_SheetCols = 4;
    int m_SheetRows = 1;
};

