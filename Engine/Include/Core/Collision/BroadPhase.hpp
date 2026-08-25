#pragma once
#include "CollisionComponent.hpp"
#include "GameFrameWork/GameplayUtils.hpp"


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

