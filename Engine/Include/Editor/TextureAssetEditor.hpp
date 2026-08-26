#pragma once
#include "Editor/AssetEditor.hpp"

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
};
