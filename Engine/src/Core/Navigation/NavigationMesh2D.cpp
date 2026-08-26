#include "Core/Navigation/NavigationMesh2D.hpp"
#include "Core/Log.h"
#include "Core/PreRequisites.h"
#include <algorithm>
#include <cmath>
#include <queue>

NavigationMesh2D &NavigationMesh2D::Get()
{
    static NavigationMesh2D s_Instance;
    return s_Instance;
}

NavigationMesh2D::NavigationMesh2D() { Build2D({-1000.0f, -1000.0f}, {1000.0f, 1000.0f}, 16.0f); }

void NavigationMesh2D::Build(const TEVector3 &worldMin, const TEVector3 &worldMax, float cellSize)
{
    Build2D({worldMin.x, worldMin.y}, {worldMax.x, worldMax.y}, cellSize);
}

void NavigationMesh2D::Build2D(const TEVector2 &worldMin, const TEVector2 &worldMax, float cellSize)
{
    m_WorldMin = worldMin;
    m_WorldMax = worldMax;
    m_CellSize = (cellSize > 1.0f) ? cellSize : 16.0f;

    float width = m_WorldMax.x - m_WorldMin.x;
    float height = m_WorldMax.y - m_WorldMin.y;

    m_GridW = (int)std::ceil(width / m_CellSize);
    m_GridH = (int)std::ceil(height / m_CellSize);

    if (m_GridW <= 0)
        m_GridW = 1;
    if (m_GridH <= 0)
        m_GridH = 1;

    m_Grid.Clear();
    m_Grid.Resize(m_GridW * m_GridH, NavCell{true, 1.0f});
    ApplyObstaclesToGrid();

    TE_CORE_INFO("[NavigationMesh2D] Built grid ({0}x{1}) with cell size {2}", m_GridW, m_GridH, m_CellSize);
}

void NavigationMesh2D::WorldToGrid(const TEVector2 &worldPos, int &outX, int &outY) const
{
    outX = (int)std::floor((worldPos.x - m_WorldMin.x) / m_CellSize);
    outY = (int)std::floor((worldPos.y - m_WorldMin.y) / m_CellSize);
}

TEVector2 NavigationMesh2D::GridToWorld(int x, int y) const
{
    return {m_WorldMin.x + (x + 0.5f) * m_CellSize, m_WorldMin.y + (y + 0.5f) * m_CellSize};
}

bool NavigationMesh2D::IsValidCell(int x, int y) const { return x >= 0 && x < m_GridW && y >= 0 && y < m_GridH; }

int NavigationMesh2D::GetCellIndex(int x, int y) const { return y * m_GridW + x; }

bool NavigationMesh2D::IsPointWalkable(const TEVector3 &point, float agentRadius) const
{
    return IsPointWalkable2D({point.x, point.y});
}

bool NavigationMesh2D::IsPointWalkable2D(const TEVector2 &point) const
{
    int gx, gy;
    WorldToGrid(point, gx, gy);
    if (!IsValidCell(gx, gy))
        return false;

    return m_Grid[GetCellIndex(gx, gy)].Walkable;
}

void NavigationMesh2D::AddObstacle(const NavObstacle &obstacle)
{
    m_Obstacles[obstacle.ID] = obstacle;
    ApplyObstaclesToGrid();
}

void NavigationMesh2D::RemoveObstacle(uint64_t obstacleId)
{
    m_Obstacles.Remove(obstacleId);
    ApplyObstaclesToGrid();
}

void NavigationMesh2D::ClearObstacles()
{
    m_Obstacles.Clear();
    ApplyObstaclesToGrid();
}

void NavigationMesh2D::AddDynamicObstacle(uint64_t id, const TEVector2 &center, const TEVector2 &size)
{
    NavObstacle obs;
    obs.ID = id;
    obs.Center = {center.x, center.y, 0.0f};
    obs.Size = {size.x, size.y, 0.0f};
    obs.bDynamic = true;
    AddObstacle(obs);
}

void NavigationMesh2D::RemoveDynamicObstacle(uint64_t id) { RemoveObstacle(id); }

void NavigationMesh2D::ApplyObstaclesToGrid()
{
    if (m_Grid.empty())
        return;

    for (auto &cell : m_Grid)
    {
        cell.Walkable = true;
        cell.Cost = 1.0f;
    }

    for (const auto &pair : m_Obstacles)
    {
        const NavObstacle &obs = pair.second;
        TEVector2 minPt = {obs.Center.x - obs.Size.x * 0.5f, obs.Center.y - obs.Size.y * 0.5f};
        TEVector2 maxPt = {obs.Center.x + obs.Size.x * 0.5f, obs.Center.y + obs.Size.y * 0.5f};

        int minGx, minGy, maxGx, maxGy;
        WorldToGrid(minPt, minGx, minGy);
        WorldToGrid(maxPt, maxGx, maxGy);

        minGx = std::max(0, minGx);
        minGy = std::max(0, minGy);
        maxGx = std::min(m_GridW - 1, maxGx);
        maxGy = std::min(m_GridH - 1, maxGy);

        for (int y = minGy; y <= maxGy; ++y)
        {
            for (int x = minGx; x <= maxGx; ++x)
            {
                m_Grid[GetCellIndex(x, y)].Walkable = false;
            }
        }
    }
}

struct NodeRecord
{
    int x, y;
    float gCost;
    float hCost;
    float fCost() const { return gCost + hCost; }

    bool operator>(const NodeRecord &other) const { return fCost() > other.fCost(); }
};

TEArray<TEVector2> NavigationMesh2D::AStarSearch(int sx, int sy, int ex, int ey)
{
    if (!IsValidCell(sx, sy) || !IsValidCell(ex, ey))
        return {};

    if (!m_Grid[GetCellIndex(ex, ey)].Walkable)
    {
        // Try adjacent walkable neighbor for target
        const int dx[] = {0, 1, 0, -1, 1, -1, 1, -1};
        const int dy[] = {1, 0, -1, 0, 1, 1, -1, -1};
        bool found = false;
        for (int i = 0; i < 8; ++i)
        {
            int nx = ex + dx[i];
            int ny = ey + dy[i];
            if (IsValidCell(nx, ny) && m_Grid[GetCellIndex(nx, ny)].Walkable)
            {
                ex = nx;
                ey = ny;
                found = true;
                break;
            }
        }
        if (!found)
            return {};
    }

    std::priority_queue<NodeRecord, TEArray<NodeRecord>, std::greater<NodeRecord>> openSet;
    TEArray<float> gCosts(m_GridW * m_GridH, std::numeric_limits<float>::infinity());
    TEArray<int> parent(m_GridW * m_GridH, -1);
    TEArray<uint8_t> closed(m_GridW * m_GridH, 0);

    int startIdx = GetCellIndex(sx, sy);
    int targetIdx = GetCellIndex(ex, ey);

    gCosts[startIdx] = 0.0f;
    float initialH = std::hypot((float)(ex - sx), (float)(ey - sy));
    openSet.push({sx, sy, 0.0f, initialH});

    const int dirX[] = {0, 1, 0, -1, 1, -1, 1, -1};
    const int dirY[] = {1, 0, -1, 0, 1, 1, -1, -1};
    const float dirCost[] = {1.0f, 1.0f, 1.0f, 1.0f, 1.4142f, 1.4142f, 1.4142f, 1.4142f};

    bool reached = false;

    while (!openSet.empty())
    {
        NodeRecord current = openSet.top();
        openSet.pop();

        int currIdx = GetCellIndex(current.x, current.y);
        if (closed[currIdx])
            continue;
        closed[currIdx] = true;

        if (current.x == ex && current.y == ey)
        {
            reached = true;
            break;
        }

        for (int i = 0; i < 8; ++i)
        {
            int nx = current.x + dirX[i];
            int ny = current.y + dirY[i];

            if (!IsValidCell(nx, ny))
                continue;

            int nIdx = GetCellIndex(nx, ny);
            if (closed[nIdx] || !m_Grid[nIdx].Walkable)
                continue;

            // Diagonal corner-cutting prevention
            if (i >= 4)
            {
                int adj1 = GetCellIndex(current.x, ny);
                int adj2 = GetCellIndex(nx, current.y);
                if (!m_Grid[adj1].Walkable || !m_Grid[adj2].Walkable)
                    continue;
            }

            float tentG = current.gCost + dirCost[i] * m_Grid[nIdx].Cost;
            if (tentG < gCosts[nIdx])
            {
                gCosts[nIdx] = tentG;
                parent[nIdx] = currIdx;
                float h = std::hypot((float)(ex - nx), (float)(ey - ny));
                openSet.push({nx, ny, tentG, h});
            }
        }
    }

    if (!reached)
        return {};

    TEArray<TEVector2> path;
    int curr = targetIdx;
    while (curr != -1)
    {
        int cy = curr / m_GridW;
        int cx = curr % m_GridW;
        path.push_back(GridToWorld(cx, cy));
        if (curr == startIdx)
            break;
        curr = parent[curr];
    }

    std::reverse(path.begin(), path.end());
    return path;
}

NavPath NavigationMesh2D::FindPath(const NavQuery &query)
{
    NavPath result;
    auto pts = FindPath2D({query.StartPos.x, query.StartPos.y}, {query.TargetPos.x, query.TargetPos.y});
    if (pts.empty())
    {
        result.bIsValid = false;
        return result;
    }

    result.bIsValid = true;
    float dist = 0.0f;
    for (size_t i = 0; i < pts.size(); ++i)
    {
        result.Waypoints.Add({pts[i].x, pts[i].y, query.StartPos.z});
        if (i > 0)
        {
            dist += (pts[i] - pts[i - 1]).Length();
        }
    }
    result.TotalDistance = dist;
    return result;
}

TEArray<TEVector2> NavigationMesh2D::FindPath2D(const TEVector2 &from, const TEVector2 &to)
{
    int sx, sy, ex, ey;
    WorldToGrid(from, sx, sy);
    WorldToGrid(to, ex, ey);

    auto stdPath = AStarSearch(sx, sy, ex, ey);
    TEArray<TEVector2> path;
    for (const auto &pt : stdPath)
    {
        path.Add(pt);
    }
    return path;
}

void NavigationMesh2D::DrawDebug() const
{
    // Debug draw walkable/blocked grid cells if renderer is active
}
