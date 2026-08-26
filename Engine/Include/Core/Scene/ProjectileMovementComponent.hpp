#pragma once
#include "Core/Scene/ComponentRegistry.hpp"
#include "MovementComponentBase.hpp"

TE_CLASS()
class TE_API ProjectileMovementComponent : public MovementComponentBase
{
public:
    TEPROPERTY()
    float InitialSpeed = 15.0f;

    TEPROPERTY()
    float GravityScale = 0.0f;

    TEPROPERTY()
    float Lifespan = 5.0f;

    TEPROPERTY()
    bool bRotationFollowsVelocity = true;

    TEPROPERTY()
    TEVector2 FlightDirection = {1.0f, 0.0f};

    ProjectileMovementComponent() = default;
    virtual ~ProjectileMovementComponent() = default;

    virtual void OnAttach() override;
    virtual void UpdateMovement(float dt) override;

    inline static const TEString StaticClassName = "ProjectileMovementComponent";
    virtual TEString GetClassName() const override { return StaticClassName; }

private:
    float m_Age = 0.0f;
};

#ifdef TE_EDITOR
T_REGISTER_COMPONENT(ProjectileMovementComponent, "Projectile Movement Component")
T_REGISTER_PROPERTY(ProjectileMovementComponent, float, InitialSpeed, "Initial Speed")
T_REGISTER_PROPERTY(ProjectileMovementComponent, float, GravityScale, "Gravity Scale")
T_REGISTER_PROPERTY(ProjectileMovementComponent, float, Lifespan, "Lifespan")
T_REGISTER_PRESET(ProjectileMovement, "Projectile Movement", "Gameplay",
                  [](EntityID id, EntityManager *em) { em->AddComponent<ProjectileMovementComponent>(id); })
#endif
