#pragma once
#include "Core/Scene/ComponentRegistry.hpp"
#include "MovementComponentBase.hpp"

enum class EPlayerMovementMode
{
    TopDown,
    SideScroller
};

TE_CLASS()
class TE_API PlayerMovementComponent2D : public MovementComponentBase
{
public:
    TEPROPERTY()
    EPlayerMovementMode MovementMode = EPlayerMovementMode::TopDown;

    TEPROPERTY()
    float Acceleration = 30.0f;

    TEPROPERTY()
    float Friction = 12.0f;

    TEPROPERTY()
    float JumpForce = 10.0f;

    TEPROPERTY()
    bool bIsGrounded = false;

    PlayerMovementComponent2D() = default;
    virtual ~PlayerMovementComponent2D() = default;

    void AddInputVector(const TEVector2 &input);
    void Jump();

    virtual void UpdateMovement(float dt) override;

    inline static const TEString StaticClassName = "PlayerMovementComponent2D";
    virtual TEString GetClassName() const override { return StaticClassName; }

private:
    TEVector2 m_InputVector = {0.0f, 0.0f};
};

#ifdef TE_EDITOR
T_REGISTER_COMPONENT(PlayerMovementComponent2D, "Player Movement 2D Component")
T_REGISTER_PROPERTY(PlayerMovementComponent2D, float, MaxSpeed, "Max Speed")
T_REGISTER_PROPERTY(PlayerMovementComponent2D, float, Acceleration, "Acceleration")
T_REGISTER_PROPERTY(PlayerMovementComponent2D, float, Friction, "Friction")
T_REGISTER_PROPERTY(PlayerMovementComponent2D, float, JumpForce, "Jump Force")
T_REGISTER_PRESET(PlayerMovement2D, "Player Movement 2D", "Gameplay",
                  [](EntityID id, EntityManager *em) { em->AddComponent<PlayerMovementComponent2D>(id); })
#endif
