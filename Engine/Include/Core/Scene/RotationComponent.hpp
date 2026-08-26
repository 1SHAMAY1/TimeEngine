#pragma once
#include "Core/Scene/ComponentRegistry.hpp"
#include "MovementComponentBase.hpp"

TE_CLASS()
class TE_API RotationComponent : public MovementComponentBase
{
public:
    TEPROPERTY()
    float RotationSpeed = 90.0f; // degrees / sec

    TEPROPERTY()
    float TargetAngle = 0.0f;

    TEPROPERTY()
    bool bAutoRotate = true;

    RotationComponent() = default;
    virtual ~RotationComponent() = default;

    virtual void UpdateMovement(float dt) override;

    inline static const TEString StaticClassName = "RotationComponent";
    virtual TEString GetClassName() const override { return StaticClassName; }
};

#ifdef TE_EDITOR
T_REGISTER_COMPONENT(RotationComponent, "Rotation Component")
T_REGISTER_PROPERTY(RotationComponent, float, RotationSpeed, "Rotation Speed")
T_REGISTER_PROPERTY(RotationComponent, float, TargetAngle, "Target Angle")
T_REGISTER_PROPERTY(RotationComponent, bool, bAutoRotate, "Auto Rotate")
T_REGISTER_PRESET(RotationComponent, "Rotation Component", "Gameplay",
                  [](EntityID id, EntityManager *em) { em->AddComponent<RotationComponent>(id); })
#endif
