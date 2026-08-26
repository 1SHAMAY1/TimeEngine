#include "DialogueRunnerPlugin.hpp"
#include "Asset/DialogueTreeAsset.hpp"
#include "Core/Asset/AssetManager.hpp"
#include "Core/Log.h"
#include "Editor/AssetEditorRegistry.hpp"
#include "Editor/DialogueGraphAssetEditor.hpp"

void DialogueRunnerPlugin::OnLoad()
{
    TE_CORE_INFO("[DialogueRunnerPlugin] Initializing...");

    // Register the .tedialogue asset type with the AssetManager
    AssetManager::RegisterAssetType(CreateRef<DialogueTreeAsset>());
    TE_CORE_INFO("[DialogueRunnerPlugin] Registered asset type: DialogueTree (.tedialogue)");

    // Register the visual graph editor into the AssetEditorRegistry
    // so ContentBrowserPanel can open .tedialogue files by double-click
    AssetEditorRegistry::Register(CreateRef<DialogueGraphAssetEditor>());
    TE_CORE_INFO("[DialogueRunnerPlugin] Registered DialogueGraphAssetEditor for asset type: DialogueTree");

    TE_CORE_INFO("[DialogueRunnerPlugin] Ready. Runtime, Visual Graph Editor, and Quest System loaded.");
}

void DialogueRunnerPlugin::OnUnload()
{
    TE_CORE_INFO("[DialogueRunnerPlugin] Unloading...");
    TE_CORE_INFO("[DialogueRunnerPlugin] Unloaded.");
}

void DialogueRunnerPlugin::DrawThumbnail(TimeGUIDrawList &dl, const TEVector2 &min, const TEVector2 &max) const
{
    unsigned int bgCol = 0xFF28183A;
    unsigned int borderCol = 0xFF9B59B6;
    dl.AddRectFilled(min, max, bgCol, 6.0f);
    dl.AddRect(min, max, borderCol, 6.0f, 0, 1.0f);

    // Left speech bubble
    TEVector2 b1Min(min.x + 7.0f, min.y + 8.0f);
    TEVector2 b1Max(min.x + 30.0f, min.y + 24.0f);
    dl.AddRectFilled(b1Min, b1Max, 0xFF3D255A, 4.0f);
    dl.AddRect(b1Min, b1Max, 0xFF00D2D3, 4.0f, 0, 1.2f);
    // Bubble tail
    TEVector2 p1(min.x + 10.0f, min.y + 24.0f), p2(min.x + 16.0f, min.y + 24.0f), p3(min.x + 8.0f, min.y + 29.0f);
    dl.AddTriangleFilled(p1, p2, p3, 0xFF00D2D3);
    // Text lines in bubble 1
    dl.AddLine(TEVector2(b1Min.x + 4.0f, b1Min.y + 5.0f), TEVector2(b1Max.x - 4.0f, b1Min.y + 5.0f), 0xFFFFFFFF, 1.5f);
    dl.AddLine(TEVector2(b1Min.x + 4.0f, b1Min.y + 10.0f), TEVector2(b1Max.x - 9.0f, b1Min.y + 10.0f), 0xFFB8E994,
               1.5f);

    // Right response bubble
    TEVector2 b2Min(min.x + 18.0f, min.y + 23.0f);
    TEVector2 b2Max(min.x + 41.0f, min.y + 38.0f);
    dl.AddRectFilled(b2Min, b2Max, 0xFF58287A, 4.0f);
    dl.AddRect(b2Min, b2Max, 0xFFE056FD, 4.0f, 0, 1.2f);
    // Response bubble tail
    TEVector2 rp1(min.x + 32.0f, min.y + 38.0f), rp2(min.x + 38.0f, min.y + 38.0f), rp3(min.x + 40.0f, min.y + 42.0f);
    dl.AddTriangleFilled(rp1, rp2, rp3, 0xFFE056FD);
    // Dialogue dots
    dl.AddCircleFilled(TEVector2(b2Min.x + 6.0f, b2Min.y + 7.5f), 1.5f, 0xFFFFFFFF);
    dl.AddCircleFilled(TEVector2(b2Min.x + 11.5f, b2Min.y + 7.5f), 1.5f, 0xFFFFFFFF);
    dl.AddCircleFilled(TEVector2(b2Min.x + 17.0f, b2Min.y + 7.5f), 1.5f, 0xFFFFFFFF);
}

TE_REGISTER_PLUGIN(DialogueRunnerPlugin)
