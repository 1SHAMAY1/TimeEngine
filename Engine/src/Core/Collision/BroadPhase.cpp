#include "Core/PreRequisites.h"
#include "Core/Collision/BroadPhase.hpp"


TEArray<CollisionPair> BroadPhase::BruteForce(TESpan<CollisionComponent *> colliders)
{
    TEArray<CollisionPair> pairs;

    for (size_t i = 0; i < colliders.size(); ++i)
    {
        for (size_t j = i + 1; j < colliders.size(); ++j)
        {
            pairs.Add({(EntityID)colliders[i]->GetOwner(), (EntityID)colliders[j]->GetOwner()});
        }
    }

    return pairs;
}

