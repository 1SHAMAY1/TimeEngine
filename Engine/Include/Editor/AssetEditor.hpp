#pragma once
#include "Core/Asset/Asset.hpp"
#include "Core/Core.h"
#include "GameFrameWork/GameplayUtils.hpp"
#include "Utils/TEString.hpp"

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
};
