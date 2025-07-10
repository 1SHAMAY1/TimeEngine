#include "Core/Collision/BroadPhase.hpp"

namespace TE {

    std::vector<CollisionPair> BroadPhase::BruteForce(
        const std::unordered_map<EntityID, CollisionComponent>& allColliders) {

        std::vector<CollisionPair> pairs;

        for (auto itA = allColliders.begin(); itA != allColliders.end(); ++itA) {
            auto itB = itA;
            ++itB;
            for (; itB != allColliders.end(); ++itB) {
                pairs.push_back({ itA->first, itB->first });
            }
        }

        return pairs;
    }

}
