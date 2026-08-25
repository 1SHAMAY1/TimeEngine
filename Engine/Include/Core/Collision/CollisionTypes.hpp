#pragma once

#include "GameFrameWork/GameplayUtils.hpp"
#include "Utils/MathUtils.hpp"
#include <cstdint>
#include <cstring>


// Type of collision shape
enum class CollisionType : uint8_t
{
    AABB = 0,
    Circle = 1,
    Triangle = 2,
    Polygon = 3,
    Capsule = 4
};

// Axis-Aligned Bounding Box
struct BoundsAABB
{
    TEVector2 min;
    TEVector2 max;

    BoundsAABB() = default;
    BoundsAABB(const TEVector2 &min, const TEVector2 &max) : min(min), max(max) {}
};

// Circle bounds
struct BoundsCircle
{
    TEVector2 center;
    float radius = 0.0f;

    BoundsCircle() = default;
    BoundsCircle(const TEVector2 &center, float radius) : center(center), radius(radius) {}
};

// Capsule bounds (2D rounded segment / pill shape)
struct BoundsCapsule
{
    TEVector2 point1;
    TEVector2 point2;
    float radius = 0.0f;

    BoundsCapsule() = default;
    BoundsCapsule(const TEVector2 &p1, const TEVector2 &p2, float r) : point1(p1), point2(p2), radius(r) {}
};

// Triangle bounds
struct BoundsTriangle
{
    TEVector2 points[3];

    BoundsTriangle() = default;
    BoundsTriangle(const TEVector2 &p1, const TEVector2 &p2, const TEVector2 &p3)
    {
        points[0] = p1;
        points[1] = p2;
        points[2] = p3;
    }
};

// Polygon bounds (Dynamic)
struct BoundsPolygon
{
    TEArray<TEVector2> points;

    BoundsPolygon() = default;
    BoundsPolygon(const TEArray<TEVector2> &p) : points(p) {}
};

// Unified collision shape (Removed union for non-POD support)
struct CollisionShape
{
    CollisionType type;

    BoundsAABB aabb;
    BoundsCircle circle;
    BoundsCapsule capsule;
    BoundsTriangle triangle;
    BoundsPolygon polygon;

    CollisionShape() : type(CollisionType::AABB) {}
    CollisionShape(const BoundsAABB &b) : type(CollisionType::AABB), aabb(b) {}
    CollisionShape(const BoundsCircle &c) : type(CollisionType::Circle), circle(c) {}
    CollisionShape(const BoundsCapsule &cp) : type(CollisionType::Capsule), capsule(cp) {}
    CollisionShape(const BoundsTriangle &t) : type(CollisionType::Triangle), triangle(t) {}
    CollisionShape(const BoundsPolygon &p) : type(CollisionType::Polygon), polygon(p) {}
};

