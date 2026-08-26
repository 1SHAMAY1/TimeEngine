#pragma once

#include "Core/PreRequisites.h"
#include "GameFrameWork/GameplayUtils.hpp"
#include "Utils/MathUtils.hpp"
#include "Utils/TEString.hpp"
#include <cstdint>

struct NavPath
{
    TEArray<TEVector3> Waypoints;
    bool bIsValid = false;
    float TotalDistance = 0.0f;

    void Clear()
    {
        Waypoints.Clear();
        bIsValid = false;
        TotalDistance = 0.0f;
    }
};

struct NavQuery
{
    TEVector3 StartPos = {0.0f, 0.0f, 0.0f};
    TEVector3 TargetPos = {0.0f, 0.0f, 0.0f};
    float AgentRadius = 16.0f;
    float AgentHeight = 32.0f;
    bool bAllowPartial = false;
};

struct NavObstacle
{
    uint64_t ID = 0;
    TEVector3 Center = {0.0f, 0.0f, 0.0f};
    TEVector3 Size = {16.0f, 16.0f, 16.0f};
    bool bDynamic = false;
};

class TE_API INavMesh
{
public:
    virtual ~INavMesh() = default;

    virtual void Build(const TEVector3 &worldMin, const TEVector3 &worldMax, float cellSize = 16.0f) = 0;
    virtual NavPath FindPath(const NavQuery &query) = 0;
    virtual bool IsPointWalkable(const TEVector3 &point, float agentRadius = 16.0f) const = 0;

    virtual void AddObstacle(const NavObstacle &obstacle) = 0;
    virtual void RemoveObstacle(uint64_t obstacleId) = 0;
    virtual void ClearObstacles() = 0;

    virtual void DrawDebug() const {}
};
