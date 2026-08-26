#include "SpriteEditorPlugin.hpp"
#include "Core/Log.h"
#include "Editor/EditorMode.hpp"
#include "SpriteMode.hpp"

void SpriteEditorPlugin::OnLoad()
{
    TE_CORE_INFO("[SpriteEditorPlugin] Loaded successfully.");
    EditorModeRegistry::RegisterMode<SpriteMode>();
}

void SpriteEditorPlugin::OnUnload()
{
    TE_CORE_INFO("[SpriteEditorPlugin] Unloading...");
    EditorModeRegistry::UnregisterMode("Sprite Mode");
}

void SpriteEditorPlugin::DrawThumbnail(TimeGUIDrawList &dl, const TEVector2 &min, const TEVector2 &max) const
{
    unsigned int bgCol = 0xFF2C141E;
    unsigned int borderCol = 0xFFFF6B6B;
    dl.AddRectFilled(min, max, bgCol, 6.0f);
    dl.AddRect(min, max, borderCol, 6.0f, 0, 1.0f);

    // 4x4 pixel art canvas grid
    float gridX = min.x + 10.0f;
    float gridY = min.y + 8.0f;
    float cell = 6.5f;

    // Pixel heart bitmap pattern (4x4)
    unsigned int heartMap[4][4] = {{0x00000000, 0xFFFF4757, 0xFFFF4757, 0x00000000},
                                   {0xFFFF4757, 0xFFFF6B81, 0xFFFF4757, 0xFFFF4757},
                                   {0xFFFF4757, 0xFFFF4757, 0xFFFF6B81, 0xFFFF4757},
                                   {0x00000000, 0xFFFF4757, 0x00000000, 0x00000000}};

    for (int r = 0; r < 4; ++r)
    {
        for (int col = 0; col < 4; ++col)
        {
            TEVector2 cMin(gridX + col * cell, gridY + r * cell);
            TEVector2 cMax(cMin.x + cell - 1.0f, cMin.y + cell - 1.0f);
            unsigned int colVal = heartMap[r][col] ? heartMap[r][col] : 0xFF3D1E28;
            dl.AddRectFilled(cMin, cMax, colVal, 1.0f);
        }
    }

    // Crosshair cursor
    TEVector2 cur(gridX + 1.0f * cell + cell * 0.5f, gridY + 1.0f * cell + cell * 0.5f);
    dl.AddRect(TEVector2(cur.x - 4.0f, cur.y - 4.0f), TEVector2(cur.x + 4.0f, cur.y + 4.0f), 0xFFFFFFFF, 0.0f, 0, 1.2f);

    // Mini palette swatches at bottom
    dl.AddRectFilled(TEVector2(min.x + 10.0f, max.y - 9.0f), TEVector2(min.x + 17.0f, max.y - 4.0f), 0xFFFF4757, 1.5f);
    dl.AddRectFilled(TEVector2(min.x + 20.0f, max.y - 9.0f), TEVector2(min.x + 27.0f, max.y - 4.0f), 0xFF2ED573, 1.5f);
    dl.AddRectFilled(TEVector2(min.x + 30.0f, max.y - 9.0f), TEVector2(min.x + 37.0f, max.y - 4.0f), 0xFF1E90FF, 1.5f);
}

TE_REGISTER_PLUGIN(SpriteEditorPlugin);
