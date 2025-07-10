#pragma once

#include <cstdint>
#include <cstring>     
#include "Utility/MathUtils.hpp" 

namespace TE {

    // Type of collision shape
    enum class CollisionType : uint8_t {
        AABB = 0,
        Circle = 1
    };

    // Axis-Aligned Bounding Box
    struct BoundsAABB {
        TEVector2 min;
        TEVector2 max;

        BoundsAABB() = default;
        BoundsAABB(const TEVector2& min, const TEVector2& max) : min(min), max(max) {}
    };

    // Circle bounds
    struct BoundsCircle {
        TEVector2 center;
        float radius = 0.0f;

        BoundsCircle() = default;
        BoundsCircle(const TEVector2& center, float radius) : center(center), radius(radius) {}
    };

    // Unified collision shape using tagged union
    struct CollisionShape {
        CollisionType type;

        union {
            BoundsAABB aabb;
            BoundsCircle circle;
        };

        // Default constructor (AABB zeroed)
        CollisionShape()
            : type(CollisionType::AABB) {
            std::memset(&aabb, 0, sizeof(BoundsAABB));
        }

        // AABB constructor
        CollisionShape(const BoundsAABB& b)
            : type(CollisionType::AABB) {
            std::memcpy(&aabb, &b, sizeof(BoundsAABB));
        }

        // Circle constructor
        CollisionShape(const BoundsCircle& c)
            : type(CollisionType::Circle) {
            std::memcpy(&circle, &c, sizeof(BoundsCircle));
        }

        // Copy constructor
        CollisionShape(const CollisionShape& other)
            : type(other.type) {
            if (type == CollisionType::AABB) {
                std::memcpy(&aabb, &other.aabb, sizeof(BoundsAABB));
            } else if (type == CollisionType::Circle) {
                std::memcpy(&circle, &other.circle, sizeof(BoundsCircle));
            }
        }

        // Assignment operator
        CollisionShape& operator=(const CollisionShape& other) {
            if (this == &other) return *this;

            type = other.type;
            if (type == CollisionType::AABB) {
                std::memcpy(&aabb, &other.aabb, sizeof(BoundsAABB));
            } else if (type == CollisionType::Circle) {
                std::memcpy(&circle, &other.circle, sizeof(BoundsCircle));
            }
            return *this;
        }

        // Destructor (safe due to POD types, but added for completeness)
        ~CollisionShape() = default;
    };

} 
