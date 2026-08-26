#include "Core/PreRequisites.h"
#include "Core/Collision/CollisionSystem.hpp"
#include "Core/Collision/CollisionTypes.hpp"
#include <algorithm>
#include <glm/glm.hpp>

void CollisionSystem::Process()
{
    if (!m_EntityManager)
        return;

    // Collect all collision components
    TEArray<CollisionComponent *> allColliders;
    for (EntityID id : m_EntityManager->GetAliveEntities())
    {
        auto colliders = m_EntityManager->GetComponents<CollisionComponent>(id);
        for (auto *col : colliders)
        {
            // Calculate world transform for this component
            TEMatrix4 worldTransform = TEMatrix4(1.0f);
            TComponent *current = col;
            while (current)
            {
                worldTransform = current->Transform.GetMatrix() * worldTransform;
                current = current->GetParentComponent();
            }

            // Update the collider shape with world-space data
            col->OnUpdateShape(worldTransform);
            allColliders.Add(col);
        }
    }

    auto pairs = BroadPhase::BruteForce(allColliders);

    for (const auto &pair : pairs)
    {
        auto collidersA = m_EntityManager->GetComponents<CollisionComponent>(pair.a);
        auto collidersB = m_EntityManager->GetComponents<CollisionComponent>(pair.b);

        for (auto *compA : collidersA)
        {
            for (auto *compB : collidersB)
            {
                if (compA->isStatic && compB->isStatic)
                    continue;

                if (CheckCollision(compA, compB))
                {
                    compA->collided = compB->collided = true;
                    if (onCollision)
                        onCollision(pair.a, pair.b);

                    // Dispatch to script instances on collision-visible components
                    auto allCompsA = m_EntityManager->GetAllComponents(pair.a);
                    for (auto *comp : allCompsA)
                    {
                        if (comp && comp->HasEventVisibility(TScriptEventType::CollisionEvent))
                        {
                            comp->DispatchScriptCollision(pair.b);
                        }
                    }

                    auto allCompsB = m_EntityManager->GetAllComponents(pair.b);
                    for (auto *comp : allCompsB)
                    {
                        if (comp && comp->HasEventVisibility(TScriptEventType::CollisionEvent))
                        {
                            comp->DispatchScriptCollision(pair.a);
                        }
                    }
                }
            }
        }
    }
}

bool CollisionSystem::CheckCollision(CollisionComponent *a, CollisionComponent *b)
{
    CollisionShape &sa = a->shape;
    CollisionShape &sb = b->shape;

    // AABB vs AABB
    if (sa.type == CollisionType::AABB && sb.type == CollisionType::AABB)
    {
        return AABBvsAABB(sa.aabb, sb.aabb);
    }
    // Circle vs Circle
    if (sa.type == CollisionType::Circle && sb.type == CollisionType::Circle)
    {
        return CircleVsCircle(sa.circle, sb.circle);
    }

    // Polygon-based SAT (Triangle is a polygon with 3 points)
    TEArray<TEVector2> pointsA, pointsB;

    auto GetPoints = [](const CollisionShape &s, TEArray<TEVector2> &out)
    {
        out.Clear();
        if (s.type == CollisionType::AABB)
        {
            out.Add({s.aabb.min.x, s.aabb.min.y});
            out.Add({s.aabb.max.x, s.aabb.min.y});
            out.Add({s.aabb.max.x, s.aabb.max.y});
            out.Add({s.aabb.min.x, s.aabb.max.y});
        }
        else if (s.type == CollisionType::Triangle)
        {
            out.Add(s.triangle.points[0]);
            out.Add(s.triangle.points[1]);
            out.Add(s.triangle.points[2]);
        }
        else if (s.type == CollisionType::Polygon)
        {
            out = s.polygon.points;
        }
        else if (s.type == CollisionType::Capsule)
        {
            TEVector2 dir = {s.capsule.point2.x - s.capsule.point1.x, s.capsule.point2.y - s.capsule.point1.y};
            float len = sqrt(dir.x * dir.x + dir.y * dir.y);
            TEVector2 norm = (len > 0.0001f)
                                 ? TEVector2{-dir.y / len * s.capsule.radius, dir.x / len * s.capsule.radius}
                                 : TEVector2{s.capsule.radius, 0.0f};
            out.Add({s.capsule.point1.x - norm.x, s.capsule.point1.y - norm.y});
            out.Add({s.capsule.point2.x - norm.x, s.capsule.point2.y - norm.y});
            out.Add({s.capsule.point2.x + norm.x, s.capsule.point2.y + norm.y});
            out.Add({s.capsule.point1.x + norm.x, s.capsule.point1.y + norm.y});
        }
    };

    if (sa.type == CollisionType::Circle || sb.type == CollisionType::Circle)
    {
        if (sa.type == CollisionType::Circle)
        {
            GetPoints(sb, pointsB);
            return CircleVsPoly(sa.circle, pointsB);
        }
        else
        {
            GetPoints(sa, pointsA);
            return CircleVsPoly(sb.circle, pointsA);
        }
    }

    GetPoints(sa, pointsA);
    GetPoints(sb, pointsB);
    return PolyVsPoly(pointsA, pointsB);
}

bool CollisionSystem::AABBvsAABB(const BoundsAABB &a, const BoundsAABB &b)
{
    return !(a.max.x < b.min.x || a.min.x > b.max.x || a.max.y < b.min.y || a.min.y > b.max.y);
}

bool CollisionSystem::CircleVsCircle(const BoundsCircle &a, const BoundsCircle &b)
{
    float r = a.radius + b.radius;
    float dx = a.center.x - b.center.x;
    float dy = a.center.y - b.center.y;
    return (dx * dx + dy * dy) <= r * r;
}

bool CollisionSystem::PolyVsPoly(TESpan<TEVector2> a, TESpan<TEVector2> b)
{
    auto axes1 = GetAxes(a);
    auto axes2 = GetAxes(b);

    for (const auto &axis : axes1)
    {
        float minA, maxA, minB, maxB;
        Project(a, axis, minA, maxA);
        Project(b, axis, minB, maxB);
        if (maxA < minB || maxB < minA)
            return false;
    }

    for (const auto &axis : axes2)
    {
        float minA, maxA, minB, maxB;
        Project(a, axis, minA, maxA);
        Project(b, axis, minB, maxB);
        if (maxA < minB || maxB < minA)
            return false;
    }

    return true;
}

bool CollisionSystem::CircleVsPoly(const BoundsCircle &circle, TESpan<TEVector2> poly)
{
    if (poly.empty())
        return false;

    // Find closest point on poly to circle center
    float minDistSq = FLT_MAX;
    TEVector2 closestPoint;

    for (size_t i = 0; i < poly.size(); i++)
    {
        TEVector2 v1 = poly[i];
        TEVector2 v2 = poly[(i + 1) % poly.size()];

        // Closest point on segment v1-v2
        TEVector2 seg = {v2.x - v1.x, v2.y - v1.y};
        TEVector2 rel = {circle.center.x - v1.x, circle.center.y - v1.y};
        float t = (rel.x * seg.x + rel.y * seg.y) / (seg.x * seg.x + seg.y * seg.y);
        t = std::max(0.0f, std::min(1.0f, t));

        TEVector2 p = {v1.x + t * seg.x, v1.y + t * seg.y};
        float dx = circle.center.x - p.x;
        float dy = circle.center.y - p.y;
        float distSq = dx * dx + dy * dy;

        if (distSq < minDistSq)
        {
            minDistSq = distSq;
            closestPoint = p;
        }
    }

    // Also check if circle center is inside poly
    bool inside = false;
    for (size_t i = 0, j = poly.size() - 1; i < poly.size(); j = i++)
    {
        if (((poly[i].y > circle.center.y) != (poly[j].y > circle.center.y)) &&
            (circle.center.x <
             (poly[j].x - poly[i].x) * (circle.center.y - poly[i].y) / (poly[j].y - poly[i].y) + poly[i].x))
        {
            inside = !inside;
        }
    }

    return inside || (minDistSq <= circle.radius * circle.radius);
}

TEArray<TEVector2> CollisionSystem::GetAxes(TESpan<TEVector2> points)
{
    TEArray<TEVector2> axes;
    for (size_t i = 0; i < points.size(); i++)
    {
        TEVector2 p1 = points[i];
        TEVector2 p2 = points[(i + 1) % points.size()];
        TEVector2 edge = {p1.x - p2.x, p1.y - p2.y};
        TEVector2 normal = {-edge.y, edge.x}; // Perpendicular
        float len = sqrt(normal.x * normal.x + normal.y * normal.y);
        if (len > 0)
            axes.Add({normal.x / len, normal.y / len});
    }
    return axes;
}

void CollisionSystem::Project(TESpan<TEVector2> points, const TEVector2 &axis, float &min, float &max)
{
    if (points.empty())
        return;
    min = max = points[0].x * axis.x + points[0].y * axis.y;
    for (size_t i = 1; i < points.size(); i++)
    {
        float p = points[i].x * axis.x + points[i].y * axis.y;
        if (p < min)
            min = p;
        else if (p > max)
            max = p;
    }
}
