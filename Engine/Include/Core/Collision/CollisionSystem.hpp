#pragma once
#include "BroadPhase.hpp"
#include "CollisionComponent.hpp"
#include "Core/Scene/EntityManager.hpp"
#include <functional>
#include <unordered_map>


class CollisionSystem
{
public:
    CollisionSystem(EntityManager *mgr) : m_EntityManager(mgr) {}

    void Process(); // Run every frame

    std::function<void(EntityID, EntityID)> onCollision;

private:
    EntityManager *m_EntityManager;

    // SAT Collision Tests
    bool CheckCollision(CollisionComponent *a, CollisionComponent *b);

    bool AABBvsAABB(const BoundsAABB &a, const BoundsAABB &b);
    bool CircleVsCircle(const BoundsCircle &a, const BoundsCircle &b);
    bool PolyVsPoly(TESpan<TEVector2> a, TESpan<TEVector2> b);
    bool CircleVsPoly(const BoundsCircle &circle, TESpan<TEVector2> poly);

    // Helper for SAT
    TEArray<TEVector2> GetAxes(TESpan<TEVector2> points);
    void Project(TESpan<TEVector2> points, const TEVector2 &axis, float &min, float &max);
};

