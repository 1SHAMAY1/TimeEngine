#pragma once

#include "Editor/AssetEditor.hpp"


class TE_API StringTableAssetEditor : public AssetEditor
{
public:
    virtual ~StringTableAssetEditor() override = default;
    virtual TEString GetAssetType() const override { return "StringTable"; }
    virtual TEString GetAssetExtension() const override { return ".testringtable"; }
    virtual TEString GetAssetCategory() const override { return "Localization"; }
    virtual TEString GetAssetDescription() const override { return "Key-value localization dictionary"; }
    virtual TEString CreateDefaultTemplate(const TEString &name) const override
    {
        return "{\n  \"StringTable\": {\n    \"Name\": \"" + name + "\",\n    \"Entries\": []\n  }\n}";
    }
    virtual void DrawEditor(EditorTab &tab) override;
    virtual void DrawIcon(const TEVector2 &min, const TEVector2 &max) const override;
};

