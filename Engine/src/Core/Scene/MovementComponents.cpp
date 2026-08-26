#include "Core/Physics/RigidBodyComponent.hpp"
#include "Core/PreRequisites.h"
#include "Core/Scene/EntityManager.hpp"
#include "Core/Scene/PlayerMovementComponent2D.hpp"
#include "Core/Scene/ProjectileMovementComponent.hpp"
#include "Core/Scene/RotationComponent.hpp"
#include <cmath>

// Helper to ensure an entity has a RigidBodyComponent
static RigidBodyComponent *EnsureRigidBody(Entity entity)
{
    if (!entity.IsValid())
        return nullptr;

    if (!entity.HasComponent<RigidBodyComponent>())
    {
        return entity.AddComponent<RigidBodyComponent>();
    }
    return entity.GetComponent<RigidBodyComponent>();
}

// ==========================================
// PlayerMovementComponent2D Implementation
// ==========================================

void PlayerMovementComponent2D::AddInputVector(const TEVector2 &input)
{
    m_InputVector.x += input.x;
    m_InputVector.y += input.y;
}

void PlayerMovementComponent2D::Jump()
{
    Entity ownerEntity = GetOwnerEntity();
    RigidBodyComponent *rb = EnsureRigidBody(ownerEntity);

    if (MovementMode == EPlayerMovementMode::SideScroller && rb)
    {
        Velocity.y = JumpForce;
        rb->Body.Velocity.y = JumpForce;
        bIsGrounded = false;
    }
}

void PlayerMovementComponent2D::UpdateMovement(float dt)
{
    if (!bMovementEnabled || dt <= 0.0f)
        return;

    Entity ownerEntity = GetOwnerEntity();
    RigidBodyComponent *rb = EnsureRigidBody(ownerEntity);
    if (!rb)
        return;

    TEVector2 targetVel = {0.0f, 0.0f};

    if (MovementMode == EPlayerMovementMode::TopDown)
    {
        if (m_InputVector.LengthSquared() > 0.001f)
        {
            m_InputVector = m_InputVector.Normalized();
            targetVel = m_InputVector * MaxSpeed;
        }

        // Smooth acceleration
        Velocity.x += (targetVel.x - Velocity.x) * std::min(1.0f, Acceleration * dt);
        Velocity.y += (targetVel.y - Velocity.y) * std::min(1.0f, Acceleration * dt);

        // Apply friction when input is zero
        if (m_InputVector.LengthSquared() <= 0.001f)
        {
            Velocity.x -= Velocity.x * std::min(1.0f, Friction * dt);
            Velocity.y -= Velocity.y * std::min(1.0f, Friction * dt);
        }
    }
    else // SideScroller
    {
        targetVel.x = m_InputVector.x * MaxSpeed;
        Velocity.x += (targetVel.x - Velocity.x) * std::min(1.0f, Acceleration * dt);

        if (m_InputVector.x == 0.0f)
        {
            Velocity.x -= Velocity.x * std::min(1.0f, Friction * dt);
        }

        // Velocity.y is driven by physics gravity
        Velocity.y = rb->Body.Velocity.y;
    }

    // Apply velocity directly to Physics RigidBody
    rb->Body.Velocity = Velocity;

    // Reset input vector for next tick
    m_InputVector = {0.0f, 0.0f};
}

// ==========================================
// ProjectileMovementComponent Implementation
// ==========================================

void ProjectileMovementComponent::OnAttach()
{
    Velocity = FlightDirection.Normalized() * InitialSpeed;
    m_Age = 0.0f;

    Entity ownerEntity = GetOwnerEntity();
    RigidBodyComponent *rb = EnsureRigidBody(ownerEntity);
    if (rb)
    {
        rb->Body.Velocity = Velocity;
    }
}

void ProjectileMovementComponent::UpdateMovement(float dt)
{
    if (!bMovementEnabled || dt <= 0.0f)
        return;

    Entity ownerEntity = GetOwnerEntity();
    RigidBodyComponent *rb = EnsureRigidBody(ownerEntity);
    if (!rb)
        return;

    m_Age += dt;
    if (Lifespan > 0.0f && m_Age >= Lifespan)
    {
        bMovementEnabled = false;
        rb->Body.Velocity = {0.0f, 0.0f};
        return;
    }

    if (GravityScale > 0.0f)
    {
        Velocity.y -= 9.81f * GravityScale * dt;
    }

    rb->Body.Velocity = Velocity;

    if (bRotationFollowsVelocity && Velocity.LengthSquared() > 0.01f)
    {
        float angleRad = std::atan2(Velocity.y, Velocity.x);
        Transform.Rotation.Roll = angleRad * (180.0f / 3.14159265f);
    }
}

// ==========================================
// RotationComponent Implementation
// ==========================================

void RotationComponent::UpdateMovement(float dt)
{
    if (!bMovementEnabled || dt <= 0.0f)
        return;

    Entity ownerEntity = GetOwnerEntity();
    RigidBodyComponent *rb = EnsureRigidBody(ownerEntity);

    if (bAutoRotate)
    {
        Transform.Rotation.Roll += RotationSpeed * dt;
        if (Transform.Rotation.Roll > 360.0f)
            Transform.Rotation.Roll -= 360.0f;
    }
    else
    {
        float current = Transform.Rotation.Roll;
        float diff = TargetAngle - current;
        if (std::abs(diff) > 0.1f)
        {
            float step = (diff > 0 ? 1.0f : -1.0f) * RotationSpeed * dt;
            if (std::abs(step) > std::abs(diff))
                step = diff;
            Transform.Rotation.Roll += step;
        }
    }
}
