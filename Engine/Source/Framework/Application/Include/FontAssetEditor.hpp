#pragma once

#include "AssetEditor.hpp"

class TE_API FontAssetEditor : public AssetEditor
{
public:
    virtual ~FontAssetEditor() override = default;
    virtual TEString GetAssetType() const override { return "Font"; }
    virtual TEString GetAssetExtension() const override { return ".tefont"; }
    virtual TEString GetAssetCategory() const override { return "UI"; }
    virtual TEString GetAssetDescription() const override { return "Baked typography glyph atlas asset"; }
    virtual TEString CreateDefaultTemplate(const TEString &name) const override
    {
        return "{\n  \"Font\": {\n    \"Name\": \"" + name + "\"\n  }\n}";
    }
    virtual void DrawEditor(EditorTab &tab) override;
    virtual void DrawIcon(const TEVector2 &min, const TEVector2 &max) const override;

    // Decentralized Import & Reimport
    virtual bool CanImportExtension(const TEString &sourceExt) const override
    {
        return sourceExt.Equals(".ttf", ESearchCase::IgnoreCase) || sourceExt.Equals(".otf", ESearchCase::IgnoreCase);
    }
    virtual TEArray<TEString> GetSupportedImportExtensions() const override { return {".ttf", ".otf"}; }
    virtual TEString GetImportTargetExtension(const TEString &sourceExt) const override { return ".tefont"; }
    virtual void OnInitImportConfig(AssetImportConfig &config) const override
    {
        config.FontSize = 32.0f;
        config.AtlasWidth = 1024;
        config.AtlasHeight = 1024;
    }
    virtual void DrawImportSettings(AssetImportConfig &config) override;
    virtual bool SupportsReimport() const override { return true; }
};

