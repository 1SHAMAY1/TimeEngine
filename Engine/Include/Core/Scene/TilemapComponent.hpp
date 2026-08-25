#pragma once

#include "Core/Asset/Asset.hpp"
#include "Core/Scene/ComponentRegistry.hpp"
#include "GameFrameWork/TComponent.hpp"
#include "Utils/MathUtils.hpp"

struct TileData
{
    int TileID = -1; // -1 = empty
    bool FlipH = false;
    bool FlipV = false;
    int RotationSteps = 0; // 0=0 deg, 1=90 deg, 2=180 deg, 3=270 deg
};

class TE_API TilemapComponent : public TComponent
{
public:
    GENERATED_BODY(TilemapComponent)

    T_PROPERTY(int, GridWidth, "Grid Width", 32)
    T_PROPERTY(int, GridHeight, "Grid Height", 32)
    T_PROPERTY(TEVector2, TileSize, "Tile Size", TEVector2(32.0f, 32.0f))
    T_PROPERTY(bool, AutoColliders, "Auto Colliders", true)

    TilemapComponent();
    virtual ~TilemapComponent() override = default;

    virtual void OnAttach() override;

    void SetTile(int x, int y, int tileID, bool flipH = false, bool flipV = false);
    TileData GetTile(int x, int y) const;
    void ClearTile(int x, int y);
    void ClearAll();

    void Resize(int newWidth, int newHeight);
    void RebuildColliders();

    void SetTileSetHandle(AssetHandle handle) { m_TileSetHandle = handle; }
    AssetHandle GetTileSetHandle() const { return m_TileSetHandle; }

    const TEArray<TileData> &GetTiles() const { return m_Tiles; }

    virtual TEString GetClassName() const override { return StaticClassName; }

private:
    AssetHandle m_TileSetHandle = 0;
    TEArray<TileData> m_Tiles;
};

#ifdef TE_EDITOR
T_REGISTER_COMPONENT(TilemapComponent, "Tilemap Component")
T_REGISTER_PROPERTY(TilemapComponent, int, GridWidth, "Grid Width")
T_REGISTER_PROPERTY(TilemapComponent, int, GridHeight, "Grid Height")
T_REGISTER_PROPERTY(TilemapComponent, TEVector2, TileSize, "Tile Size")
T_REGISTER_PROPERTY(TilemapComponent, bool, AutoColliders, "Auto Colliders")
T_REGISTER_PRESET(TilemapComponent, "Tilemap Layer", "2D Rendering",
                  [](EntityID id, EntityManager *em) { em->AddComponent<TilemapComponent>(id); })
#endif
