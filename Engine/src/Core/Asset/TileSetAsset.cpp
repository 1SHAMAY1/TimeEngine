#include "Core/Asset/TileSetAsset.hpp"
#include "Core/Asset/AssetRegistry.hpp"
#include "Core/PreRequisites.h"
#include "Utils/TEFileSystem.hpp"
#include <fstream>

TileSetAsset::TileSetAsset() {}

TileSetAsset::TileSetAsset(const TEString &name) : m_Name(name) {}

TERef<Asset> TileSetAsset::Clone() const
{
    auto clone = CreateRef<TileSetAsset>(m_Name);
    clone->m_Handle = m_Handle;
    clone->m_TextureAtlasHandle = m_TextureAtlasHandle;
    clone->m_Tiles = m_Tiles;
    return clone;
}

bool TileSetAsset::LoadFromFile(const TEString &path)
{
    if (!TEFileSystem::Exists(path))
        return false;

    m_Handle = AssetRegistry::RegisterPath(path);
    m_Tiles.Clear();

    return TEFileSystem::ForEachLine(path,
                                     [this](const TEString &line)
                                     {
                                         TEString trimmed = line.Trim();
                                         if (trimmed.empty() || trimmed.StartsWith("#"))
                                             return true;

                                         if (trimmed.StartsWith("Atlas: "))
                                         {
                                             m_TextureAtlasHandle =
                                                 static_cast<AssetHandle>(trimmed.Mid(7).Trim().ToInt64());
                                         }
                                         else if (trimmed.StartsWith("Tile: "))
                                         {
                                             TEString content = trimmed.Mid(6);
                                             auto parts = content.Split(" ");
                                             if (parts.Num() >= 6)
                                             {
                                                 TileSetEntry entry;
                                                 entry.TileID = parts[0].ToInt();
                                                 entry.UVRect.x = parts[1].ToFloat();
                                                 entry.UVRect.y = parts[2].ToFloat();
                                                 entry.UVRect.z = parts[3].ToFloat();
                                                 entry.UVRect.w = parts[4].ToFloat();
                                                 entry.IsCollidable = (parts[5].ToInt() != 0);
                                                 if (parts.Num() > 6)
                                                     entry.Tag = parts[6];
                                                 m_Tiles.Add(entry);
                                             }
                                         }
                                         return true;
                                     });
}

bool TileSetAsset::SaveToFile(const TEString &path)
{
    std::ofstream file(path.c_str());
    if (!file.is_open())
        return false;

    file << "# TimeEngine TileSet Asset\n";
    file << "Atlas: " << m_TextureAtlasHandle << "\n";
    for (size_t i = 0; i < m_Tiles.Num(); ++i)
    {
        const auto &t = m_Tiles[i];
        file << "Tile: " << t.TileID << " " << t.UVRect.x << " " << t.UVRect.y << " " << t.UVRect.z << " " << t.UVRect.w
             << " " << (t.IsCollidable ? 1 : 0) << " " << t.Tag.c_str() << "\n";
    }
    return true;
}

void TileSetAsset::OnContentBrowserCreate(const TEString &path)
{
    TEString baseName = "NewTileSet";
    TEString finalPath = path / (baseName + ".tetileset");
    int counter = 1;
    while (TEFileSystem::Exists(finalPath))
    {
        finalPath = path / (baseName + "_" + TEString::FromInt(counter++) + ".tetileset");
    }

    SaveToFile(finalPath);
}

const TileSetEntry *TileSetAsset::FindTile(int tileID) const
{
    for (size_t i = 0; i < m_Tiles.Num(); ++i)
    {
        if (m_Tiles[i].TileID == tileID)
            return &m_Tiles[i];
    }
    return nullptr;
}
