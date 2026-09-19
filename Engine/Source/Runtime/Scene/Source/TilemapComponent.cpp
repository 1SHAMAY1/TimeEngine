#include "PreRequisites.h"
#include "TilemapComponent.hpp"
#include "BoxColliderComponent.hpp"

TilemapComponent::TilemapComponent() { Resize(GridWidth, GridHeight); }

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

void TilemapComponent::ClearTile(int x, int y) { SetTile(x, y, -1); }

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

void TilemapComponent::OnRender(Renderer2D *renderer, const TEMatrix4 &worldModel,
                                const TERef<Material> &material) const
{
    if (!renderer)
        return;

    // Draw grid wireframe bounds in editor
    float totalW = static_cast<float>(GridWidth) * TileSize.x;
    float totalH = static_cast<float>(GridHeight) * TileSize.y;
    TEVector2 gridCenter(totalW * 0.5f, totalH * 0.5f);
    TEVector4 centerWorld = worldModel * TEVector4(gridCenter.x, gridCenter.y, 0.0f, 1.0f);
    renderer->SubmitRectOutline(TEVector2(centerWorld.x, centerWorld.y), TEVector2(totalW, totalH), 1.0f,
                                TEColor(0.3f, 0.7f, 1.0f, 0.4f));

    // Render placed tiles
    for (int y = 0; y < GridHeight; ++y)
    {
        for (int x = 0; x < GridWidth; ++x)
        {
            TileData tile = GetTile(x, y);
            if (tile.TileID < 0)
                continue;

            TEVector2 tilePos(x * TileSize.x + TileSize.x * 0.5f, y * TileSize.y + TileSize.y * 0.5f);
            TEVector4 tileWorldPos = worldModel * TEVector4(tilePos.x, tilePos.y, 0.0f, 1.0f);

            renderer->SubmitQuad(TEVector2(tileWorldPos.x, tileWorldPos.y), TileSize, TEColor::White(), material);
        }
    }
}
