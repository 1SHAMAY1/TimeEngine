#pragma once
#include "CollisionComponent.hpp"
#include "GameplayUtils.hpp"

using EntityID = uint64_t;

struct CollisionPair
{
    EntityID a, b;
};

class BroadPhase
{
public:
    static TEArray<CollisionPair> BruteForce(TESpan<CollisionComponent *> colliders);
};
