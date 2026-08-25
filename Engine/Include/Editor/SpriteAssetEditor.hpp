#pragma once
#include "Editor/AssetEditor.hpp"


class TE_API SpriteAssetEditor : public AssetEditor
{
public:
    virtual ~SpriteAssetEditor() override = default;
    virtual TEString GetAssetType() const override { return "Sprite"; }
    virtual TEString GetAssetExtension() const override { return ".tesprite"; }
    virtual TEString GetAssetCategory() const override { return "2D Art"; }
    virtual TEString GetAssetDescription() const override { return "Sprite asset with custom UV boundaries and colliders"; }
    virtual TEString CreateDefaultTemplate(const TEString &name) const override
    {
        return "{\n  \"Sprite\": {\n    \"Name\": \"" + name + "\",\n    \"Texture\": \"\",\n    \"U0\": 0.0, \"V0\": 0.0,\n    \"U1\": 1.0, \"V1\": 1.0\n  }\n}";
    }
    virtual void DrawEditor(EditorTab &tab) override;
    virtual void DrawIcon(const TEVector2 &min, const TEVector2 &max) const override;
};

