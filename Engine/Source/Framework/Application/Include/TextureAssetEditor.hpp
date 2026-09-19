#pragma once
#include "AssetEditor.hpp"

class TE_API TextureAssetEditor : public AssetEditor
{
public:
    virtual ~TextureAssetEditor() override = default;
    virtual TEString GetAssetType() const override { return "Texture"; }
    virtual TEString GetAssetExtension() const override { return ".tetexture"; }
    virtual TEArray<TEString> GetSupportedExtensions() const override { return {".tetexture"}; }
    virtual TEString GetAssetCategory() const override { return "Textures"; }
    virtual void DrawEditor(EditorTab &tab) override;
    virtual void DrawIcon(const TEVector2 &min, const TEVector2 &max) const override;

    // Decentralized Import & Reimport
    virtual bool CanImportExtension(const TEString &sourceExt) const override;
    virtual TEArray<TEString> GetSupportedImportExtensions() const override;
    virtual TEString GetImportTargetExtension(const TEString &sourceExt) const override { return ".tetexture"; }
    virtual void OnInitImportConfig(AssetImportConfig &config) const override;
    virtual void DrawImportSettings(AssetImportConfig &config) override;
    virtual bool SupportsReimport() const override { return true; }
};
