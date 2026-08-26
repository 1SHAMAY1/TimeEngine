#include "Core/Scene/RayCast2DComponent.hpp"
#include "Core/PreRequisites.h"
#include "Core/Scene/TransformComponent.hpp"

void RayCast2DComponent::Tick(float deltaTime)
{
    // Runtime tick
}

bool RayCast2DComponent::CastRay(PhysicsWorld *physicsWorld)
{
    if (!Enabled || !physicsWorld)
    {
        IsHitting = false;
        HitEntityID = 0;
        return false;
    }

    auto *transform = GetOwnerEntity().GetComponent<TransformComponent>();
    if (!transform)
    {
        IsHitting = false;
        HitEntityID = 0;
        return false;
    }

    TEVector2 start = {transform->Transform.Position.x, transform->Transform.Position.y};
    TEVector2 dir = Direction;
    float len = TEVector2::Length(dir);
    if (len > 0.0001f)
    {
        dir = dir / len;
    }
    else
    {
        dir = {0.0f, 1.0f};
    }

    uint32_t outEntityID = 0;
    IsHitting = physicsWorld->Raycast(start, dir, Length, HitPoint, HitNormal, HitFraction, outEntityID);
    HitEntityID = static_cast<EntityID>(outEntityID);

    return IsHitting;
}
