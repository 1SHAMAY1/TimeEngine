#include "RichTextPlugin.hpp"
#include "Core/Asset/AssetManager.hpp"
#include "Core/Log.h"
#include "RichTextDataAsset.hpp"
#include "RichTextTable.hpp"

void RichTextPlugin::OnLoad()
{
    TE_CORE_INFO("[RichTextPlugin] Initializing...");

    // Register .terichtext (RichTextTable) and RichTextDataAsset asset types
    AssetManager::RegisterAssetType(CreateRef<RichTextTable>());
    AssetManager::RegisterAssetType(CreateRef<RichTextDataAsset>());
    TE_CORE_INFO("[RichTextPlugin] Registered asset types: RichTextTable (.terichtext) & RichTextDataAsset");

    TE_CORE_INFO("[RichTextPlugin] Ready. Rich text parser, table styles, animators, and layout loaded.");
}

void RichTextPlugin::OnUnload()
{
    TE_CORE_INFO("[RichTextPlugin] Unloading...");
    TE_CORE_INFO("[RichTextPlugin] Unloaded.");
}

void RichTextPlugin::DrawThumbnail(TimeGUIDrawList &dl, const TEVector2 &min, const TEVector2 &max) const
{
    float w = max.x - min.x;
    float h = max.y - min.y;
    TEVector2 c = TEVector2(min.x + w * 0.5f, min.y + h * 0.5f);

    unsigned int bgCol = 0xFF0E1A30;
    unsigned int borderCol = 0xFF74B9FF;
    dl.AddRectFilled(min, max, bgCol, 6.0f);
    dl.AddRect(min, max, borderCol, 6.0f, 0, 1.0f);

    // Typographic serif 'A'
    TEVector2 aTop(c.x, min.y + 10.0f);
    TEVector2 aLeft(min.x + 12.0f, max.y - 12.0f);
    TEVector2 aRight(max.x - 12.0f, max.y - 12.0f);

    dl.AddLine(aTop, aLeft, 0xFF0984E3, 3.5f);
    dl.AddLine(aTop, aRight, 0xFF74B9FF, 3.5f);
    dl.AddLine(TEVector2(min.x + 17.0f, c.y + 4.0f), TEVector2(max.x - 17.0f, c.y + 4.0f), 0xFFFFFFFF, 2.5f);

    // Baseline underline
    dl.AddLine(TEVector2(min.x + 9.0f, max.y - 8.0f), TEVector2(max.x - 9.0f, max.y - 8.0f), 0xFF00CEC9, 2.0f);

    // Formatting color swatch dots
    dl.AddCircleFilled(TEVector2(min.x + 36.0f, min.y + 12.0f), 2.5f, 0xFFFF4757);
    dl.AddCircleFilled(TEVector2(min.x + 36.0f, min.y + 18.0f), 2.5f, 0xFF2ED573);
}

TE_REGISTER_PLUGIN(RichTextPlugin)
