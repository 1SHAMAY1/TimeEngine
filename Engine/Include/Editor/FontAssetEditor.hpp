#pragma once

#include "Editor/AssetEditor.hpp"

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
};
