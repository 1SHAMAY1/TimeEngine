#pragma once
#include "Asset.hpp"
#include "Core.h"
#include "GameplayUtils.hpp"
#include "EngineTypes/TEString.hpp"

#include "AssetManager.hpp"

struct EditorTab
{
    TEString Title;
    TEString AssetPath;
    TEString Type;
    TERef<Asset> LoadedAsset;
};

class TE_API AssetEditor
{
public:
    virtual ~AssetEditor() = default;
    virtual TEString GetAssetType() const = 0;
    virtual TEString GetAssetExtension() const { return ""; }
    virtual TEArray<TEString> GetSupportedExtensions() const { return {}; }
    virtual TEString GetAssetCategory() const { return "General"; }
    virtual TEString GetAssetDescription() const { return ""; }
    virtual TEString CreateDefaultTemplate(const TEString &name) const { return ""; }
    virtual void DrawEditor(EditorTab &tab) = 0;
    virtual void DrawIcon(const TEVector2 &min, const TEVector2 &max) const {}

    // Decentralized Import & Reimport hooks
    virtual bool CanImportExtension(const TEString &sourceExt) const { return false; }
    virtual TEArray<TEString> GetSupportedImportExtensions() const { return {}; }
    virtual TEString GetImportTargetExtension(const TEString &sourceExt) const { return GetAssetExtension(); }
    virtual void OnInitImportConfig(AssetImportConfig &config) const {}
    virtual void DrawImportSettings(AssetImportConfig &config) {}
    virtual bool SupportsReimport() const { return false; }
};
