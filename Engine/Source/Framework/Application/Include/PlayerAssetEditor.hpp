#pragma once
#include "PlayerAsset.hpp"
#include "AssetEditor.hpp"
#include "Widgets/UICodeEdit.hpp"

class TE_API PlayerAssetEditor : public AssetEditor
{
public:
    PlayerAssetEditor();
    virtual ~PlayerAssetEditor() override = default;

    virtual TEString GetAssetType() const override { return "Player"; }
    virtual TEString GetAssetExtension() const override { return ".teplayer"; }
    virtual TEArray<TEString> GetSupportedExtensions() const override { return {".teplayer"}; }
    virtual TEString GetAssetCategory() const override { return "Gameplay"; }
    virtual TEString GetAssetDescription() const override { return "Player Asset: Defines player appearance, physics attributes, and movement characteristics."; }

    virtual void DrawEditor(EditorTab &tab) override;
    virtual void DrawIcon(const TEVector2 &min, const TEVector2 &max) const override;

private:
    UICodeEdit m_CodeEdit;
    TEString m_LastLoadedPath = "";
};
