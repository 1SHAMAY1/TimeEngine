#pragma once

#include "Editor/AssetEditor.hpp"

class TE_API TScriptAssetEditor : public AssetEditor
{
public:
    virtual ~TScriptAssetEditor() override = default;
    virtual TEString GetAssetType() const override { return "TScript"; }
    virtual TEString GetAssetExtension() const override { return ".tscript"; }
    virtual TEString GetAssetCategory() const override { return "Scripting"; }
    virtual TEString GetAssetDescription() const override { return "TimeEngine script component with lifecycle hooks"; }
    virtual TEString CreateDefaultTemplate(const TEString &name) const override
    {
        return "class " + name + " {\n    on_ready() {\n    }\n\n    on_update(dt: float) {\n    }\n}\n";
    }
    virtual void DrawEditor(EditorTab &tab) override;
    virtual void DrawIcon(const TEVector2 &min, const TEVector2 &max) const override;
};
