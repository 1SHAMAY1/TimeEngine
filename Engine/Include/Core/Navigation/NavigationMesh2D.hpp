#pragma once

#include "Core/Navigation/INavMesh.hpp"
#include "GameFrameWork/GameplayUtils.hpp"

class TE_API NavigationMesh2D : public INavMesh
{
public:
    static NavigationMesh2D &Get();

    NavigationMesh2D();
    virtual ~NavigationMesh2D() override = default;

    virtual void Build(const TEVector3 &worldMin, const TEVector3 &worldMax, float cellSize = 16.0f) override;
    void Build2D(const TEVector2 &worldMin, const TEVector2 &worldMax, float cellSize = 16.0f);

    virtual NavPath FindPath(const NavQuery &query) override;
    TEArray<TEVector2> FindPath2D(const TEVector2 &from, const TEVector2 &to);

    virtual bool IsPointWalkable(const TEVector3 &point, float agentRadius = 16.0f) const override;
    bool IsPointWalkable2D(const TEVector2 &point) const;

    virtual void AddObstacle(const NavObstacle &obstacle) override;
    virtual void RemoveObstacle(uint64_t obstacleId) override;
    virtual void ClearObstacles() override;

    void AddDynamicObstacle(uint64_t id, const TEVector2 &center, const TEVector2 &size);
    void RemoveDynamicObstacle(uint64_t id);

    virtual void DrawDebug() const override;

    int GetGridWidth() const { return m_GridW; }
    int GetGridHeight() const { return m_GridH; }
    float GetCellSize() const { return m_CellSize; }

private:
    struct NavCell
    {
        bool Walkable = true;
        float Cost = 1.0f;
    };

    int m_GridW = 0;
    int m_GridH = 0;
    float m_CellSize = 16.0f;
    TEVector2 m_WorldMin = {0.0f, 0.0f};
    TEVector2 m_WorldMax = {0.0f, 0.0f};

    TEArray<NavCell> m_Grid;
    TEMap<uint64_t, NavObstacle> m_Obstacles;

    void WorldToGrid(const TEVector2 &worldPos, int &outX, int &outY) const;
    TEVector2 GridToWorld(int x, int y) const;
    bool IsValidCell(int x, int y) const;
    int GetCellIndex(int x, int y) const;

    TEArray<TEVector2> AStarSearch(int sx, int sy, int ex, int ey);
    void ApplyObstaclesToGrid();
};
