#include "Core/Collision/BroadPhase.hpp"

namespace TE {

    std::vector<CollisionPair> BroadPhase::BruteForce(const std::vector<CollisionComponent*>& colliders) {

        std::vector<CollisionPair> pairs;

        for (size_t i = 0; i < colliders.size(); ++i) {
            for (size_t j = i + 1; j < colliders.size(); ++j) {
                pairs.push_back({ (EntityID)colliders[i]->GetOwner(), (EntityID)colliders[j]->GetOwner() });
            }
        }

        return pairs;
    }

}
