#pragma once

#include "AssetEditor.hpp"

class TE_API DataTableAssetEditor : public AssetEditor
{
public:
    virtual ~DataTableAssetEditor() override = default;
    virtual TEString GetAssetType() const override { return "DataTable"; }
    virtual TEString GetAssetExtension() const override { return ".tedatatable"; }
    virtual TEString GetAssetCategory() const override { return "Data"; }
    virtual TEString GetAssetDescription() const override { return "Structured tabular spreadsheet data"; }
    virtual TEString CreateDefaultTemplate(const TEString &name) const override
    {
        return "{\n  \"DataTable\": {\n    \"Name\": \"" + name + "\",\n    \"Columns\": [],\n    \"Rows\": []\n  }\n}";
    }
    virtual void DrawEditor(EditorTab &tab) override;
    virtual void DrawIcon(const TEVector2 &min, const TEVector2 &max) const override;

    // Decentralized Import & Reimport
    virtual bool CanImportExtension(const TEString &sourceExt) const override
    {
        return sourceExt.Equals(".csv", ESearchCase::IgnoreCase);
    }
    virtual TEArray<TEString> GetSupportedImportExtensions() const override { return {".csv"}; }
    virtual TEString GetImportTargetExtension(const TEString &sourceExt) const override { return ".tedatatable"; }
    virtual void OnInitImportConfig(AssetImportConfig &config) const override
    {
        config.IsDataTable = true;
    }
    virtual void DrawImportSettings(AssetImportConfig &config) override;
    virtual bool SupportsReimport() const override { return true; }
};

