#pragma once
#include "PlayerControllerAsset.hpp"
#include "AssetEditor.hpp"
#include "Widgets/UICodeEdit.hpp"

class TE_API PlayerControllerAssetEditor : public AssetEditor
{
public:
    PlayerControllerAssetEditor();
    virtual ~PlayerControllerAssetEditor() override = default;

    virtual TEString GetAssetType() const override { return "PlayerController"; }
    virtual TEString GetAssetExtension() const override { return ".tecontroller"; }
    virtual TEArray<TEString> GetSupportedExtensions() const override { return {".tecontroller"}; }
    virtual TEString GetAssetCategory() const override { return "Gameplay"; }
    virtual TEString GetAssetDescription() const override { return "Player Controller Asset: Defines input routing and action evaluation via InputMappingContext."; }

    virtual void DrawEditor(EditorTab &tab) override;
    virtual void DrawIcon(const TEVector2 &min, const TEVector2 &max) const override;

private:
    UICodeEdit m_CodeEdit;
    TEString m_LastLoadedPath = "";
};
