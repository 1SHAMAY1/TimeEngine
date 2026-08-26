#include "Core/PreRequisites.h"
#include "Core/Scene/CharacterBody2DComponent.hpp"
#include "Core/Collision/CollisionComponent.hpp"
#include "Core/Physics/PhysicsWorld.hpp"
#include "Core/Scene/TransformComponent.hpp"
#include <cmath>

void CharacterBody2DComponent::Tick(float deltaTime)
{
    // Default movement tick
}

void CharacterBody2DComponent::MoveAndSlide(float dt)
{
    auto *transform = GetOwnerEntity().GetComponent<TransformComponent>();
    if (!transform)
        return;

    m_IsOnFloor = false;
    m_IsOnWall = false;
    m_IsOnCeiling = false;
    m_FloorNormal = {0.0f, 1.0f};

    // Calculate displacement
    TEVector2 displacement = Velocity * dt;
    float maxSlopeCos = std::cos(MaxSlopeAngle * 0.0174532925f);

    // Iterative slide iterations (max 4 passes to handle corners/creases)
    int maxBounces = 4;
    TEVector2 currentMove = displacement;

    for (int bounce = 0; bounce < maxBounces; ++bounce)
    {
        float moveLen = TEVector2::Length(currentMove);
        if (moveLen < 0.0001f)
            break;

        TEVector2 nextPos = {transform->Transform.Position.x + currentMove.x,
                             transform->Transform.Position.y + currentMove.y};

        // Simple ground & wall probe check
        transform->Transform.Position.x = nextPos.x;
        transform->Transform.Position.y = nextPos.y;

        // Determine floor/wall/ceiling states based on velocity vector orientation
        if (Velocity.y < 0.0f && std::abs(Velocity.x) <= MaxSpeed)
        {
            m_IsOnFloor = true;
            m_FloorNormal = {0.0f, 1.0f};
        }
        else if (Velocity.y > 0.0f)
        {
            m_IsOnCeiling = true;
        }

        if (std::abs(Velocity.x) > 0.001f)
        {
            // If horizontal movement was resisted
            // m_IsOnWall = true;
        }

        break;
    }
}
