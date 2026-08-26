#pragma once

#include "Core/PreRequisites.h"
#include "Editor/AssetEditor.hpp"
#include "Editor/Graph/NodeCanvas.hpp"
#include "Editor/MaterialPreviewViewport.hpp"

class MaterialAssetEditor : public AssetEditor
{
public:
    MaterialAssetEditor();
    virtual ~MaterialAssetEditor() override = default;

    virtual TEString GetAssetType() const override { return "MaterialAsset"; }
    virtual TEString GetAssetExtension() const override { return ".tematerial"; }
    virtual TEString GetAssetCategory() const override { return "Rendering"; }
    virtual TEString GetAssetDescription() const override { return "Modular node-based shader graph material"; }
    virtual TEString CreateDefaultTemplate(const TEString &name) const override
    {
        return "{\n  \"Material\": {\n    \"Name\": \"" + name +
               "\",\n    \"Shader\": \"DefaultPBR\",\n    \"Nodes\": []\n  }\n}";
    }
    virtual void DrawEditor(EditorTab &tab) override;
    virtual void DrawIcon(const TEVector2 &min, const TEVector2 &max) const override;

private:
    void DrawDetailsPanel(Graph &graph, const TERef<GraphNode> &selectedNode);

    NodeCanvas m_Canvas;
    MaterialPreviewViewport m_PreviewViewport;
    bool m_PaletteInitialized = false;
};
