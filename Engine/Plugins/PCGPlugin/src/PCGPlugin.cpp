#include "PCGPlugin.hpp"
#include "Asset/PCGGraphAsset.hpp"
#include "Core/Asset/AssetManager.hpp"
#include "Core/Log.h"
#include "ECS/PCGVolumeComponent.hpp"
#include "Editor/AssetEditorRegistry.hpp"
#include "Editor/PCGGraphAssetEditor.hpp"

void PCGPlugin::OnLoad()
{
    TE_CORE_INFO("[PCGPlugin] Initializing...");

    // Register .tepcg Asset Type
    AssetManager::RegisterAssetType(CreateRef<PCGGraphAsset>());
    TE_CORE_INFO("[PCGPlugin] Registered Asset Type: PCGGraph (.tepcg)");

    // Register PCGGraphAssetEditor into AssetEditorRegistry
    AssetEditorRegistry::Register(CreateRef<PCGGraphAssetEditor>());
    TE_CORE_INFO("[PCGPlugin] Registered PCGGraphAssetEditor for asset type: PCGGraph");

    TE_CORE_INFO("[PCGPlugin] PCG Toolset, 12 Specialized Processors, and Asset Editor ready.");
}

void PCGPlugin::OnUnload()
{
    TE_CORE_INFO("[PCGPlugin] Unloading...");
    TE_CORE_INFO("[PCGPlugin] Unloaded.");
}

void PCGPlugin::DrawThumbnail(TimeGUIDrawList &dl, const TEVector2 &min, const TEVector2 &max) const
{
    unsigned int bgCol = 0xFF0A261A;
    unsigned int borderCol = 0xFF2ECC71;
    dl.AddRectFilled(min, max, bgCol, 6.0f);
    dl.AddRect(min, max, borderCol, 6.0f, 0, 1.0f);

    // Mountain terrain contour
    TEVector2 m1[4] = {
        TEVector2(min.x + 8.0f, max.y - 10.0f),
        TEVector2(min.x + 20.0f, min.y + 14.0f),
        TEVector2(min.x + 32.0f, max.y - 10.0f),
        TEVector2(min.x + 8.0f, max.y - 10.0f)
    };
    dl.AddConvexPolyFilled(m1, 3, 0xFF1B6E44);
    dl.AddPolyline(m1, 3, 0xFF2ECC71, 0, 1.5f);

    TEVector2 m2[4] = {
        TEVector2(min.x + 22.0f, max.y - 10.0f),
        TEVector2(min.x + 33.0f, min.y + 18.0f),
        TEVector2(max.x - 6.0f, max.y - 10.0f),
        TEVector2(min.x + 22.0f, max.y - 10.0f)
    };
    dl.AddConvexPolyFilled(m2, 3, 0xFF218C53);
    dl.AddPolyline(m2, 3, 0xFF55EFC4, 0, 1.5f);

    // Seed generator sparkle
    TEVector2 sp(min.x + 14.0f, min.y + 12.0f);
    dl.AddLine(TEVector2(sp.x - 4.0f, sp.y), TEVector2(sp.x + 4.0f, sp.y), 0xFFFFFFFF, 1.2f);
    dl.AddLine(TEVector2(sp.x, sp.y - 4.0f), TEVector2(sp.x, sp.y + 4.0f), 0xFFFFFFFF, 1.2f);
}

TE_REGISTER_PLUGIN(PCGPlugin)
