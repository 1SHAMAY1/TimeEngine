#include "Core/Collision/CollisionSystem.hpp"
#include <glm/glm.hpp>
#include "Core/Collision/CollisionTypes.hpp"

namespace TE {

    void CollisionSystem::Register(EntityID id, const CollisionComponent& comp) {
        m_Colliders[id] = comp;
    }

    void CollisionSystem::Unregister(EntityID id) {
        m_Colliders.erase(id);
    }

    void CollisionSystem::Clear() {
        m_Colliders.clear();
    }

    bool CollisionSystem::AABBvsAABB(const BoundsAABB& a, const BoundsAABB& b) {
        return !(a.max.x < b.min.x || a.min.x > b.max.x ||
                 a.max.y < b.min.y || a.min.y > b.max.y);
    }

    bool CollisionSystem::CircleVsCircle(const BoundsCircle& a, const BoundsCircle& b) {
        float r = a.radius + b.radius;
        return Distance(a.center, b.center) <= r;
    }

    void CollisionSystem::Process() {
        auto pairs = BroadPhase::BruteForce(m_Colliders);

        for (const auto& pair : pairs) {
            auto& compA = m_Colliders[pair.a];
            auto& compB = m_Colliders[pair.b];

            if (compA.isStatic && compB.isStatic) continue;

            bool hit = false;

            if (compA.shape.type == CollisionType::AABB &&
                compB.shape.type == CollisionType::AABB) {
                hit = AABBvsAABB(compA.shape.aabb, compB.shape.aabb);
                } else if (compA.shape.type == CollisionType::Circle &&
                           compB.shape.type == CollisionType::Circle) {
                    hit = CircleVsCircle(compA.shape.circle, compB.shape.circle);
                           }

            // You can expand this for AABB vs Circle support

            if (hit) {
                compA.collided = compB.collided = true;
                if (onCollision) onCollision(pair.a, pair.b);
            }
        }
    }

}
