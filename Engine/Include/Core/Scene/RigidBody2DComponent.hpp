#pragma once

#include "Core/Physics/PhysicsWorld.hpp"
#include "Core/Scene/ComponentRegistry.hpp"
#include "GameFrameWork/TComponent.hpp"
#include "Utils/MathUtils.hpp"

enum class ERigidBodyType2D : uint8_t
{
    Static = 0,
    Kinematic = 1,
    Dynamic = 2
};

class TE_API RigidBody2DComponent : public TComponent
{
public:
    GENERATED_BODY(RigidBody2DComponent)
    T_EVENT_VISIBLE(TScriptEventType::CollisionEvent)

    T_PROPERTY(int, BodyType, "Body Type", static_cast<int>(ERigidBodyType2D::Dynamic))
    T_PROPERTY(float, Mass, "Mass", 1.0f)
    T_PROPERTY(float, GravityScale, "Gravity Scale", 1.0f)
    T_PROPERTY(float, LinearDamping, "Linear Damping", 0.0f)
    T_PROPERTY(float, AngularDamping, "Angular Damping", 0.01f)
    T_PROPERTY(float, Restitution, "Restitution", 0.5f)
    T_PROPERTY(bool, FixedRotation, "Fixed Rotation", false)

    RigidBody2DComponent();
    virtual ~RigidBody2DComponent() override = default;

    virtual void OnAttach() override;
    virtual void OnDetach() override;
    virtual void Tick(float deltaTime) override;

    // Transform synchronization with PhysicsWorld
    void SyncToPhysics(PhysicsWorld *physicsWorld);
    void SyncFromPhysics(PhysicsWorld *physicsWorld);

    // Dynamic control API
    void SetLinearVelocity(const TEVector2 &velocity);
    TEVector2 GetLinearVelocity() const;

    void SetAngularVelocity(float omega);
    float GetAngularVelocity() const;

    void ApplyForce(const TEVector2 &force);
    void ApplyImpulse(const TEVector2 &impulse, const TEVector2 &worldPoint = {0.0f, 0.0f});

    ERigidBodyType2D GetRigidBodyType() const { return static_cast<ERigidBodyType2D>(BodyType); }
    void SetRigidBodyType(ERigidBodyType2D type) { BodyType = static_cast<int>(type); }

    virtual TEString GetClassName() const override { return StaticClassName; }

    RigidBody &GetInternalBody() { return m_InternalBody; }
    const RigidBody &GetInternalBody() const { return m_InternalBody; }

private:
    RigidBody m_InternalBody;
    float m_AngularVelocity = 0.0f;
};

#ifdef TE_EDITOR
T_REGISTER_COMPONENT(RigidBody2DComponent, "RigidBody 2D Component")
T_REGISTER_PROPERTY(RigidBody2DComponent, int, BodyType, "Body Type")
T_REGISTER_PROPERTY(RigidBody2DComponent, float, Mass, "Mass")
T_REGISTER_PROPERTY(RigidBody2DComponent, float, GravityScale, "Gravity Scale")
T_REGISTER_PROPERTY(RigidBody2DComponent, float, LinearDamping, "Linear Damping")
T_REGISTER_PROPERTY(RigidBody2DComponent, float, AngularDamping, "Angular Damping")
T_REGISTER_PROPERTY(RigidBody2DComponent, float, Restitution, "Restitution")
T_REGISTER_PROPERTY(RigidBody2DComponent, bool, FixedRotation, "Fixed Rotation")
T_REGISTER_PRESET(RigidBody2DComponent, "RigidBody 2D", "Physics & Collisions",
                  [](EntityID id, EntityManager *em) { em->AddComponent<RigidBody2DComponent>(id); })
#endif
