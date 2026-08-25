#pragma once

#include "Core/Asset/Asset.hpp"
#include "GameFrameWork/GameplayUtils.hpp"
#include "Utils/MathUtils.hpp"
#include "Utils/TEString.hpp"

struct TileSetEntry
{
    int TileID = 0;
    TEVector4 UVRect = {0.0f, 0.0f, 1.0f, 1.0f};
    bool IsCollidable = false;
    TEString Tag = "Ground";
};

class TE_API TileSetAsset : public Asset
{
public:
    TileSetAsset();
    TileSetAsset(const TEString &name);
    virtual ~TileSetAsset() override = default;

    virtual AssetHandle GetHandle() const override { return m_Handle; }
    virtual const TEString &GetType() const override { return m_AssetTypeName; }
    virtual const TEString &GetName() const override { return m_Name; }
    virtual const TEString &GetHoverDescription() const override { return m_Description; }

    virtual TEString GetDefaultExtension() const override { return ".tetileset"; }
    virtual TEString GetDefaultIconPath() const override { return "Resources/Editor/FileIcon.png"; }

    virtual TERef<Asset> Clone() const override;
    virtual bool LoadFromFile(const TEString &path) override;
    virtual bool SaveToFile(const TEString &path);
    virtual void OnContentBrowserCreate(const TEString &path) override;

    void SetName(const TEString &name) { m_Name = name; }
    void SetHandle(AssetHandle handle) { m_Handle = handle; }

    AssetHandle GetTextureAtlasHandle() const { return m_TextureAtlasHandle; }
    void SetTextureAtlasHandle(AssetHandle handle) { m_TextureAtlasHandle = handle; }

    const TEArray<TileSetEntry> &GetTiles() const { return m_Tiles; }
    void AddTile(const TileSetEntry &entry) { m_Tiles.Add(entry); }
    const TileSetEntry *FindTile(int tileID) const;

private:
    AssetHandle m_Handle = 0;
    TEString m_Name = "NewTileSet";
    TEString m_AssetTypeName = "TileSetAsset";
    TEString m_Description = "TimeEngine 2D TileSet Asset";

    AssetHandle m_TextureAtlasHandle = 0;
    TEArray<TileSetEntry> m_Tiles;
};
