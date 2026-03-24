#pragma once
#include "CollisionComponent.hpp"
#include <vector>

namespace TE
{

using EntityID = uint64_t;

struct CollisionPair
{
    EntityID a, b;
};

class BroadPhase
{
public:
    static std::vector<CollisionPair> BruteForce(const std::vector<CollisionComponent *> &colliders);
};

} // namespace TE
