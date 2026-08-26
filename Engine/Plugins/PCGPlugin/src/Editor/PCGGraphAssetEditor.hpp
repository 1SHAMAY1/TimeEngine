#pragma once

#include "Asset/PCGGraphAsset.hpp"
#include "Core/PreRequisites.h"
#include "Editor/AssetEditor.hpp"
#include "Editor/Graph/NodeCanvas.hpp"
#include "GameFrameWork/GameplayUtils.hpp"

class PCGGraphAssetEditor : public AssetEditor
{
public:
    PCGGraphAssetEditor();
    virtual ~PCGGraphAssetEditor() override = default;

    virtual TEString GetAssetType() const override { return "PCGGraph"; }
    virtual TEString GetAssetExtension() const override { return ".tepcg"; }
    virtual TEString GetAssetCategory() const override { return "Procedural"; }
    virtual TEString GetAssetDescription() const override { return "Procedural content generation node rules"; }
    virtual TEString CreateDefaultTemplate(const TEString &name) const override
    {
        return "{\n  \"PCGGraph\": {\n    \"Name\": \"" + name + "\",\n    \"Nodes\": []\n  }\n}";
    }
    virtual void DrawEditor(EditorTab &tab) override;
    virtual void DrawIcon(const TEVector2 &min, const TEVector2 &max) const override;

private:
    void DrawToolbar(PCGGraphAsset &asset, const TEString &assetPath);
    void DrawNodeInspector(Graph &graph);

    NodeCanvas m_Canvas;
    bool m_PaletteRegistered = false;
    TEMap<TEString, TERef<PCGGraphAsset>> m_LoadedAssets;
    size_t m_LastGeneratedPointCount = 0;
};
