#pragma once

#include "Core/Asset/StateTreeAsset.hpp"
#include "Editor/AssetEditor.hpp"
#include "Editor/Graph/NodeCanvas.hpp"

class TE_API StateTreeAssetEditor : public AssetEditor
{
public:
    StateTreeAssetEditor();
    virtual ~StateTreeAssetEditor() override = default;

    virtual TEString GetAssetType() const override { return "StateTreeAsset"; }
    virtual TEString GetAssetExtension() const override { return ".testatetree"; }
    virtual TEString GetAssetCategory() const override { return "AI"; }
    virtual TEString GetAssetDescription() const override { return "Hierarchical state machine decision tree"; }
    virtual TEString CreateDefaultTemplate(const TEString &name) const override
    {
        return "{\n  \"StateTree\": {\n    \"Name\": \"" + name + "\",\n    \"States\": []\n  }\n}";
    }
    virtual void DrawEditor(EditorTab &tab) override;
    virtual void DrawIcon(const TEVector2 &min, const TEVector2 &max) const override;

private:
    NodeCanvas m_Canvas;
    uint64_t m_SelectedNodeID = 0;
};
