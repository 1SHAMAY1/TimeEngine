#pragma once

#include "Editor/AssetEditor.hpp"


class TE_API DataAssetEditor : public AssetEditor
{
public:
    virtual ~DataAssetEditor() override = default;
    virtual TEString GetAssetType() const override { return "DataAsset"; }
    virtual TEString GetAssetExtension() const override { return ".tedataasset"; }
    virtual TEString GetAssetCategory() const override { return "Data"; }
    virtual TEString GetAssetDescription() const override { return "Generic typed property data asset"; }
    virtual TEString CreateDefaultTemplate(const TEString &name) const override
    {
        return "{\n  \"DataAsset\": {\n    \"Name\": \"" + name + "\",\n    \"Properties\": {}\n  }\n}";
    }
    virtual void DrawEditor(EditorTab &tab) override;
    virtual void DrawIcon(const TEVector2 &min, const TEVector2 &max) const override;
};

