#pragma once
#include "CollisionComponent.hpp"
#include "BroadPhase.hpp"
#include <unordered_map>
#include <functional>

namespace TE {

    class CollisionSystem {
    public:
        using EntityID = uint32_t;

        void Register(EntityID id, const CollisionComponent& comp);
        void Unregister(EntityID id);
        void Clear();
        void Process();  // Run every frame

        std::function<void(EntityID, EntityID)> onCollision;

    private:
        std::unordered_map<EntityID, CollisionComponent> m_Colliders;

        bool AABBvsAABB(const BoundsAABB& a, const BoundsAABB& b);
        bool CircleVsCircle(const BoundsCircle& a, const BoundsCircle& b);
    };

}
