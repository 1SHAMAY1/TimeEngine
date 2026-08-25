#include "MaterialSystemPlugin.hpp"
#include "Asset/MaterialAsset.hpp"
#include "Asset/MaterialInstanceAsset.hpp"
#include "Core/Asset/AssetManager.hpp"
#include "Core/Log.h"
#include "Editor/AssetEditorRegistry.hpp"
#include "Editor/MaterialAssetEditor.hpp"
#include "Editor/MaterialInstanceEditor.hpp"
#include "Integration/MaterialMCPTools.hpp"


void MaterialSystemPlugin::OnLoad()
{
    TE_CORE_INFO("[MaterialSystemPlugin] Initializing Modular Material System...");

    // Register .tematerial and .tematinst asset types with AssetManager
    AssetManager::RegisterAssetType(CreateRef<MaterialAsset>());
    AssetManager::RegisterAssetType(CreateRef<MaterialInstanceAsset>());
    TE_CORE_INFO("[MaterialSystemPlugin] Registered Asset Types: MaterialAsset (.tematerial), MaterialInstanceAsset (.tematinst)");

    // Register visual editors
    AssetEditorRegistry::Register(CreateRef<MaterialAssetEditor>());
    AssetEditorRegistry::Register(CreateRef<MaterialInstanceEditor>());
    TE_CORE_INFO("[MaterialSystemPlugin] Registered MaterialAssetEditor & MaterialInstanceEditor");

    // Initialize MCP tools if present
    MaterialMCPTools::RegisterTools();

    TE_CORE_INFO("[MaterialSystemPlugin] Ready.");
}

void MaterialSystemPlugin::OnUnload()
{
    TE_CORE_INFO("[MaterialSystemPlugin] Unloading...");
    MaterialMCPTools::UnregisterTools();
    TE_CORE_INFO("[MaterialSystemPlugin] Unloaded.");
}

void MaterialSystemPlugin::DrawThumbnail(TimeGUIDrawList &dl, const TEVector2 &min, const TEVector2 &max) const
{
    float w = max.x - min.x;
    float h = max.y - min.y;
    TEVector2 c = TEVector2(min.x + w * 0.5f, min.y + h * 0.5f);

    unsigned int bgCol = 0xFF280E18;
    unsigned int borderCol = 0xFFFF4757;
    dl.AddRectFilled(min, max, bgCol, 6.0f);
    dl.AddRect(min, max, borderCol, 6.0f, 0, 1.0f);

    // 3D-shaded material sphere
    dl.AddCircleFilled(c, 14.0f, 0xFF4A1024);
    dl.AddCircleFilled(c, 12.0f, 0xFFB33939);
    dl.AddCircleFilled(TEVector2(c.x - 2.0f, c.y - 2.0f), 9.0f, 0xFFFF5252);
    dl.AddCircleFilled(TEVector2(c.x - 5.0f, c.y - 5.0f), 4.5f, 0xFFFFB142);
    dl.AddCircleFilled(TEVector2(c.x - 6.0f, c.y - 6.0f), 2.0f, 0xFFFFFFFF);

    // Shader connection pins
    dl.AddCircleFilled(TEVector2(min.x + 6.0f, c.y - 6.0f), 2.5f, 0xFFFF7675);
    dl.AddCircleFilled(TEVector2(min.x + 6.0f, c.y + 6.0f), 2.5f, 0xFF55EFC4);
    dl.AddCircleFilled(TEVector2(max.x - 6.0f, c.y), 2.5f, 0xFF00D2D3);
}

TE_REGISTER_PLUGIN(MaterialSystemPlugin)
