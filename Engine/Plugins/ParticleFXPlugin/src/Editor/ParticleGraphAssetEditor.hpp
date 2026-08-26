#pragma once

#include "Core/PreRequisites.h"
#include "Editor/AssetEditor.hpp"
#include "Editor/Graph/NodeCanvas.hpp"

class ParticleGraphAssetEditor : public AssetEditor
{
public:
    ParticleGraphAssetEditor();
    virtual ~ParticleGraphAssetEditor() override = default;

    virtual TEString GetAssetType() const override { return "ParticleSystemAsset"; }
    virtual TEString GetAssetExtension() const override { return ".teparticle"; }
    virtual TEString GetAssetCategory() const override { return "VFX"; }
    virtual TEString GetAssetDescription() const override { return "Modular particle emission and simulation graph"; }
    virtual TEString CreateDefaultTemplate(const TEString &name) const override
    {
        return "{\n  \"ParticleSystem\": {\n    \"Name\": \"" + name + "\",\n    \"Emitters\": []\n  }\n}";
    }
    virtual void DrawEditor(EditorTab &tab) override;
    virtual void DrawIcon(const TEVector2 &min, const TEVector2 &max) const override;

private:
    NodeCanvas m_Canvas;
    Graph m_Graph;
    bool m_Initialized = false;
};
