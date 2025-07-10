#pragma once
#include "CollisionComponent.hpp"
#include <vector>
#include <unordered_map>

namespace TE {

    using EntityID = uint32_t;

    struct CollisionPair {
        EntityID a, b;
    };

    class BroadPhase {
    public:
        static std::vector<CollisionPair> BruteForce(
            const std::unordered_map<EntityID, CollisionComponent>& allColliders);
    };

}
