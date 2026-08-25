#pragma once
#include "GameFrameWork/TComponent.hpp"
#include "Utils/MathUtils.hpp"


TE_CLASS()
class TE_API MovementComponentBase : public TComponent
{
public:
    TEPROPERTY()
    TEVector2 Velocity = {0.0f, 0.0f};

    TEPROPERTY()
    float MaxSpeed = 10.0f;

    TEPROPERTY()
    bool bMovementEnabled = true;

    MovementComponentBase() = default;
    virtual ~MovementComponentBase() = default;

    virtual void UpdateMovement(float dt) {}
    virtual void Tick(float deltaTime) override { UpdateMovement(deltaTime); }

    inline static const TEString StaticClassName = "MovementComponentBase";
    virtual TEString GetClassName() const override { return StaticClassName; }
};

