#pragma once

#include "Core/Asset/Asset.hpp"
#include "GameFrameWork/GameplayUtils.hpp"
#include "Renderer/Texture.hpp"
#include "Utils/MathUtils.hpp"
#include "Utils/TEString.hpp"



struct TE_API FontGlyph
{
    TEString Character = " ";
    float AdvanceX = 0.0f;
    float BearingX = 0.0f;
    float BearingY = 0.0f;
    float Width = 0.0f;
    float Height = 0.0f;
    TEVector4 UV{0.0f, 0.0f, 0.0f, 0.0f}; // u0, v0, u1, v1
};

class TE_API FontAsset : public Asset
{
public:
    FontAsset();
    FontAsset(const TEString &name, float pixelSize = 32.0f);
    virtual ~FontAsset() = default;

    // Asset Overrides
    virtual AssetHandle GetHandle() const override { return m_Handle; }
    virtual const TEString &GetType() const override { return m_AssetTypeName; }
    virtual const TEString &GetName() const override { return m_Name; }
    virtual const TEString &GetHoverDescription() const override { return m_Description; }

    virtual TEString GetDefaultExtension() const override { return ".tefont"; }
    virtual TEString GetDefaultIconPath() const override { return "Resources/Editor/FileIcon.png"; }

    virtual TERef<Asset> Clone() const override;
    virtual bool LoadFromFile(const TEString &path) override;
    virtual bool SaveToFile(const TEString &path);
    virtual void OnContentBrowserCreate(const TEString &path) override;

    // Font Atlas Baking
    bool BakeFromTTF(const TEString &ttfPath, float pixelSize = 32.0f,
                     uint32_t atlasWidth = 512, uint32_t atlasHeight = 512);

    // Typography Metrics & Query
    bool GetGlyph(const TEString &character, FontGlyph &outGlyph) const;
    FontGlyph GetGlyph(const TEString &character) const;
    bool HasGlyph(const TEString &character) const;
    float GetKerning(const TEString &first, const TEString &second) const;
    void SetKerning(const TEString &first, const TEString &second, float kern);
    float GetLineHeight() const { return m_LineHeight; }
    float GetAscent() const { return m_Ascent; }
    float GetDescent() const { return m_Descent; }
    float GetPixelSize() const { return m_PixelSize; }
    uint32_t GetAtlasWidth() const { return m_AtlasWidth; }
    uint32_t GetAtlasHeight() const { return m_AtlasHeight; }

    const TEString &GetSourcePath() const { return m_SourcePath; }
    const TEString &GetAtlasTexturePath() const { return m_AtlasTexturePath; }
    TERef<Texture> GetAtlasTexture();

    const TEMap<TEString, FontGlyph> &GetGlyphs() const { return m_Glyphs; }

    float MeasureStringWidth(const TEString &text, float scale = 1.0f) const;
    TEVector2 MeasureString(const TEString &text, float scale = 1.0f) const;

    void SetName(const TEString &name) { m_Name = name; }
    void SetHandle(AssetHandle handle) { m_Handle = handle; }

protected:
    AssetHandle m_Handle = 0;
    TEString m_Name = "NewFont";
    TEString m_AssetTypeName = "Font";
    TEString m_Description = "TimeEngine vector TrueType/OpenType font asset";

    TEString m_SourcePath;
    TEString m_AtlasTexturePath;
    float m_PixelSize = 32.0f;
    uint32_t m_AtlasWidth = 512;
    uint32_t m_AtlasHeight = 512;
    float m_LineHeight = 36.0f;
    float m_Ascent = 28.0f;
    float m_Descent = -8.0f;

    TEMap<TEString, FontGlyph> m_Glyphs;
    TEMap<TEString, float> m_Kerning;

    TERef<Texture> m_AtlasTexture;
};

