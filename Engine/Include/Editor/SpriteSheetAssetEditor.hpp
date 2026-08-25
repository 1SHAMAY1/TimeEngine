#pragma once
#include "Editor/AssetEditor.hpp"


class TE_API SpriteSheetAssetEditor : public AssetEditor
{
public:
    virtual ~SpriteSheetAssetEditor() override = default;
    virtual TEString GetAssetType() const override { return "SpriteSheet"; }
    virtual TEString GetAssetExtension() const override { return ".tesheet"; }
    virtual TEString GetAssetCategory() const override { return "2D Art"; }
    virtual TEString GetAssetDescription() const override { return "Multi-frame animated spritesheet definition"; }
    virtual TEString CreateDefaultTemplate(const TEString &name) const override
    {
        return "{\n  \"SpriteSheet\": {\n    \"Name\": \"" + name + "\",\n    \"Texture\": \"\",\n    \"Frames\": [],\n    \"Animations\": []\n  }\n}";
    }
    virtual void DrawEditor(EditorTab &tab) override;
    virtual void DrawIcon(const TEVector2 &min, const TEVector2 &max) const override;
};

