#pragma once

#include "AssetEditor.hpp"
#include "Widgets/UICodeEdit.hpp"

class TE_API TScriptAssetEditor : public AssetEditor
{
public:
    TScriptAssetEditor();
    virtual ~TScriptAssetEditor() override = default;
    virtual TEString GetAssetType() const override { return "TScript"; }
    virtual TEString GetAssetExtension() const override { return ".tscript"; }
    virtual TEString GetAssetCategory() const override { return "Scripting"; }
    virtual TEString GetAssetDescription() const override { return "TimeEngine script component with lifecycle hooks"; }
    virtual TEString CreateDefaultTemplate(const TEString &name) const override
    {
        return "class " + name + " : TComponent {\n    on_ready() {\n    }\n\n    on_update(float dt) {\n    }\n}\n";
    }
    virtual void DrawEditor(EditorTab &tab) override;
    virtual void DrawIcon(const TEVector2 &min, const TEVector2 &max) const override;

private:
    UICodeEdit m_CodeEdit;
    TEString m_LastLoadedPath;
    TEString m_FilterBuffer;
    bool m_ShowAPIBrowser = true;
};
