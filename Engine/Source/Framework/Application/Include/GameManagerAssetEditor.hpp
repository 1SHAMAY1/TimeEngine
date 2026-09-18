#pragma once
#include "GameManagerAsset.hpp"
#include "AssetEditor.hpp"
#include "Widgets/UICodeEdit.hpp"

class TE_API GameManagerAssetEditor : public AssetEditor
{
public:
    GameManagerAssetEditor();
    virtual ~GameManagerAssetEditor() override = default;

    virtual TEString GetAssetType() const override { return "GameManager"; }
    virtual TEString GetAssetExtension() const override { return ".tegamemanager"; }
    virtual TEArray<TEString> GetSupportedExtensions() const override { return {".tegamemanager"}; }
    virtual TEString GetAssetCategory() const override { return "Gameplay"; }
    virtual TEString GetAssetDescription() const override { return "Game Manager Asset: Defines rules, player spawning, and controller bindings."; }

    virtual void DrawEditor(EditorTab &tab) override;
    virtual void DrawIcon(const TEVector2 &min, const TEVector2 &max) const override;

private:
    UICodeEdit m_CodeEdit;
    TEString m_LastLoadedPath = "";
};
