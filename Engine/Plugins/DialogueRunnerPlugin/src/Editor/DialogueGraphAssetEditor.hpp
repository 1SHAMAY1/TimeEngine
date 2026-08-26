#pragma once

#include "Asset/DialogueTreeAsset.hpp"
#include "Editor/AssetEditor.hpp"
#include "Editor/AssetEditorRegistry.hpp"
#include "Editor/DialogueGraphCanvas.hpp"
#include "Editor/DialogueLiveSimulator.hpp"
#include "Editor/DialogueNodeInspector.hpp"

class DialogueGraphAssetEditor : public AssetEditor
{
public:
    DialogueGraphAssetEditor() = default;
    virtual ~DialogueGraphAssetEditor() = default;

    virtual TEString GetAssetType() const override { return "DialogueTree"; }
    virtual TEString GetAssetExtension() const override { return ".tedialogue"; }
    virtual TEString GetAssetCategory() const override { return "Narrative"; }
    virtual TEString GetAssetDescription() const override { return "Interactive narrative dialogue graph"; }
    virtual TEString CreateDefaultTemplate(const TEString &name) const override
    {
        return "{\n  \"DialogueTree\": {\n    \"Name\": \"" + name + "\",\n    \"Nodes\": []\n  }\n}";
    }
    virtual void DrawEditor(EditorTab &tab) override;
    virtual void DrawIcon(const TEVector2 &min, const TEVector2 &max) const override;

private:
    void DrawToolbar(DialogueTreeAsset &asset, const TEString &assetPath);
    void Save(DialogueTreeAsset &asset, const TEString &assetPath);

    DialogueGraphCanvas m_Canvas;
    DialogueNodeInspector m_Inspector;
    DialogueLiveSimulator m_Simulator;

    bool m_bShowSimulator = false;
};
