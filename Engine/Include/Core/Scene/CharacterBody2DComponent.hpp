#pragma once

#include "Core/Scene/ComponentRegistry.hpp"
#include "Core/Scene/MovementComponentBase.hpp"
#include "Utils/MathUtils.hpp"

class TE_API CharacterBody2DComponent : public MovementComponentBase
{
public:
    GENERATED_BODY(CharacterBody2DComponent)
    T_EVENT_VISIBLE(TScriptEventType::CollisionEvent)

    T_PROPERTY(float, MaxSpeed, "Max Speed", 300.0f)
    T_PROPERTY(float, Gravity, "Gravity", 980.0f)
    T_PROPERTY(float, MaxSlopeAngle, "Max Slope Angle", 45.0f)
    T_PROPERTY(float, StepHeight, "Step Height", 10.0f)
    T_PROPERTY(float, FloorSnapLength, "Floor Snap Length", 8.0f)

    TEVector2 Velocity = {0.0f, 0.0f};

    CharacterBody2DComponent() = default;
    virtual ~CharacterBody2DComponent() override = default;

    virtual void Tick(float deltaTime) override;

    // Core movement resolution
    void MoveAndSlide(float dt);

    bool IsOnFloor() const { return m_IsOnFloor; }
    bool IsOnWall() const { return m_IsOnWall; }
    bool IsOnCeiling() const { return m_IsOnCeiling; }
    TEVector2 GetFloorNormal() const { return m_FloorNormal; }

    void SetVelocity(const TEVector2 &vel) { Velocity = vel; }
    TEVector2 GetVelocity() const { return Velocity; }

    virtual TEString GetClassName() const override { return StaticClassName; }

private:
    bool m_IsOnFloor = false;
    bool m_IsOnWall = false;
    bool m_IsOnCeiling = false;
    TEVector2 m_FloorNormal = {0.0f, 1.0f};
};

#ifdef TE_EDITOR
T_REGISTER_COMPONENT(CharacterBody2DComponent, "CharacterBody 2D Component")
T_REGISTER_PROPERTY(CharacterBody2DComponent, float, MaxSpeed, "Max Speed")
T_REGISTER_PROPERTY(CharacterBody2DComponent, float, Gravity, "Gravity")
T_REGISTER_PROPERTY(CharacterBody2DComponent, float, MaxSlopeAngle, "Max Slope Angle")
T_REGISTER_PROPERTY(CharacterBody2DComponent, float, StepHeight, "Step Height")
T_REGISTER_PROPERTY(CharacterBody2DComponent, float, FloorSnapLength, "Floor Snap Length")
T_REGISTER_PRESET(CharacterBody2DComponent, "Character Body 2D", "Physics & Collisions",
                  [](EntityID id, EntityManager *em) { em->AddComponent<CharacterBody2DComponent>(id); })
#endif
