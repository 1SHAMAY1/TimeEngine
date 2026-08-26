#pragma once

#include "Core/Physics/PhysicsWorld.hpp"
#include "Core/Scene/ComponentRegistry.hpp"
#include "GameFrameWork/TComponent.hpp"
#include "Utils/MathUtils.hpp"

class TE_API RayCast2DComponent : public TComponent
{
public:
    GENERATED_BODY(RayCast2DComponent)

    T_PROPERTY(TEVector2, Direction, "Direction", TEVector2(0.0f, 1.0f))
    T_PROPERTY(float, Length, "Length", 100.0f)
    T_PROPERTY(bool, Enabled, "Enabled", true)

    // Raycast query results
    bool IsHitting = false;
    EntityID HitEntityID = 0;
    TEVector2 HitPoint = {0.0f, 0.0f};
    TEVector2 HitNormal = {0.0f, 0.0f};
    float HitFraction = 0.0f;

    RayCast2DComponent() = default;
    virtual ~RayCast2DComponent() override = default;

    virtual void Tick(float deltaTime) override;

    bool CastRay(PhysicsWorld *physicsWorld);

    virtual TEString GetClassName() const override { return StaticClassName; }
};

#ifdef TE_EDITOR
T_REGISTER_COMPONENT(RayCast2DComponent, "RayCast 2D Component")
T_REGISTER_PROPERTY(RayCast2DComponent, TEVector2, Direction, "Direction")
T_REGISTER_PROPERTY(RayCast2DComponent, float, Length, "Length")
T_REGISTER_PROPERTY(RayCast2DComponent, bool, Enabled, "Enabled")
T_REGISTER_PRESET(RayCast2DComponent, "RayCast 2D", "Physics & Collisions",
                  [](EntityID id, EntityManager *em) { em->AddComponent<RayCast2DComponent>(id); })
#endif
