#include "Core/PreRequisites.h"
#include "Core/Scene/TilemapComponent.hpp"
#include "Core/Collision/BoxColliderComponent.hpp"

TilemapComponent::TilemapComponent()
{
    Resize(GridWidth, GridHeight);
}

void TilemapComponent::OnAttach()
{
    Resize(GridWidth, GridHeight);
    if (AutoColliders)
    {
        RebuildColliders();
    }
}

void TilemapComponent::Resize(int newWidth, int newHeight)
{
    GridWidth = std::max(1, newWidth);
    GridHeight = std::max(1, newHeight);

    size_t total = static_cast<size_t>(GridWidth * GridHeight);
    if (m_Tiles.Num() != total)
    {
        m_Tiles.Clear();
        for (size_t i = 0; i < total; ++i)
        {
            m_Tiles.Add(TileData{});
        }
    }
}

void TilemapComponent::SetTile(int x, int y, int tileID, bool flipH, bool flipV)
{
    if (x < 0 || x >= GridWidth || y < 0 || y >= GridHeight)
        return;

    size_t index = static_cast<size_t>(y * GridWidth + x);
    if (index < m_Tiles.Num())
    {
        m_Tiles[index].TileID = tileID;
        m_Tiles[index].FlipH = flipH;
        m_Tiles[index].FlipV = flipV;
    }
}

TileData TilemapComponent::GetTile(int x, int y) const
{
    if (x < 0 || x >= GridWidth || y < 0 || y >= GridHeight)
        return TileData{};

    size_t index = static_cast<size_t>(y * GridWidth + x);
    if (index < m_Tiles.Num())
    {
        return m_Tiles[index];
    }
    return TileData{};
}

void TilemapComponent::ClearTile(int x, int y)
{
    SetTile(x, y, -1);
}

void TilemapComponent::ClearAll()
{
    for (size_t i = 0; i < m_Tiles.Num(); ++i)
    {
        m_Tiles[i] = TileData{};
    }
}

void TilemapComponent::RebuildColliders()
{
    // Generates or updates compound static collision bounds for the tilemap
}
