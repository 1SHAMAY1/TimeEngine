#include "Core/PreRequisites.h"
#include "Core/Scene/RigidBody2DComponent.hpp"
#include "Core/Scene/TransformComponent.hpp"
#include "Core/Scene/Scene.hpp"

RigidBody2DComponent::RigidBody2DComponent()
{
    m_InternalBody.Mass = Mass;
    m_InternalBody.InverseMass = (Mass > 0.0f) ? 1.0f / Mass : 0.0f;
    m_InternalBody.Restitution = Restitution;
    m_InternalBody.IsStatic = (GetRigidBodyType() == ERigidBodyType2D::Static);
}

void RigidBody2DComponent::OnAttach()
{
    m_InternalBody.m_VeloxEntityID = static_cast<uint32_t>(GetOwnerEntity().GetID());
    m_InternalBody.Mass = Mass;
    m_InternalBody.InverseMass = (Mass > 0.0f && GetRigidBodyType() == ERigidBodyType2D::Dynamic) ? 1.0f / Mass : 0.0f;
    m_InternalBody.Restitution = Restitution;
    m_InternalBody.IsStatic = (GetRigidBodyType() == ERigidBodyType2D::Static);

    auto *transform = GetOwnerEntity().GetComponent<TransformComponent>();
    if (transform)
    {
        m_InternalBody.Position = {transform->Transform.Position.x, transform->Transform.Position.y};
    }
}

void RigidBody2DComponent::OnDetach()
{
    // Cleanup if needed
}

void RigidBody2DComponent::Tick(float deltaTime)
{
    // Apply damping
    if (GetRigidBodyType() == ERigidBodyType2D::Dynamic)
    {
        if (LinearDamping > 0.0f)
        {
            m_InternalBody.Velocity *= std::max(0.0f, 1.0f - LinearDamping * deltaTime);
        }
        if (AngularDamping > 0.0f)
        {
            m_AngularVelocity *= std::max(0.0f, 1.0f - AngularDamping * deltaTime);
        }
    }
}

void RigidBody2DComponent::SyncToPhysics(PhysicsWorld *physicsWorld)
{
    auto *transform = GetOwnerEntity().GetComponent<TransformComponent>();
    if (transform)
    {
        m_InternalBody.Position = {transform->Transform.Position.x, transform->Transform.Position.y};
        m_InternalBody.IsStatic = (GetRigidBodyType() == ERigidBodyType2D::Static);
        m_InternalBody.Mass = Mass;
        m_InternalBody.InverseMass = (Mass > 0.0f && GetRigidBodyType() == ERigidBodyType2D::Dynamic) ? 1.0f / Mass : 0.0f;
    }
}

void RigidBody2DComponent::SyncFromPhysics(PhysicsWorld *physicsWorld)
{
    if (GetRigidBodyType() == ERigidBodyType2D::Dynamic)
    {
        auto *transform = GetOwnerEntity().GetComponent<TransformComponent>();
        if (transform)
        {
            transform->Transform.Position.x = m_InternalBody.Position.x;
            transform->Transform.Position.y = m_InternalBody.Position.y;

            if (!FixedRotation && std::abs(m_AngularVelocity) > 0.0001f)
            {
                transform->Transform.Rotation.Roll += m_AngularVelocity;
            }
        }
    }
}

void RigidBody2DComponent::SetLinearVelocity(const TEVector2 &velocity)
{
    m_InternalBody.Velocity = velocity;
}

TEVector2 RigidBody2DComponent::GetLinearVelocity() const
{
    return m_InternalBody.Velocity;
}

void RigidBody2DComponent::SetAngularVelocity(float omega)
{
    m_AngularVelocity = omega;
}

float RigidBody2DComponent::GetAngularVelocity() const
{
    return m_AngularVelocity;
}

void RigidBody2DComponent::ApplyForce(const TEVector2 &force)
{
    if (GetRigidBodyType() == ERigidBodyType2D::Dynamic)
    {
        m_InternalBody.ApplyForce(force * GravityScale);
    }
}

void RigidBody2DComponent::ApplyImpulse(const TEVector2 &impulse, const TEVector2 &worldPoint)
{
    if (GetRigidBodyType() == ERigidBodyType2D::Dynamic && m_InternalBody.InverseMass > 0.0f)
    {
        m_InternalBody.Velocity += impulse * m_InternalBody.InverseMass;
    }
}
