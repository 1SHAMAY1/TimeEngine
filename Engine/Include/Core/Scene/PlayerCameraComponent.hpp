#pragma once
#include "Core/Scene/ComponentRegistry.hpp"
#include "GameFrameWork/TComponent.hpp"
#include "Utils/MathUtils.hpp"

TE_CLASS()
class TE_API PlayerCameraComponent : public TComponent
{
public:
    TEPROPERTY()
    bool bFollowOwner = true;

    TEPROPERTY()
    float SmoothSpeed = 8.0f;

    TEPROPERTY()
    TEVector2 FollowOffset = {0.0f, 0.0f};

    TEPROPERTY()
    float OrthographicSize = 5.0f;

    PlayerCameraComponent();
    virtual ~PlayerCameraComponent() = default;

    virtual void OnAttach() override;
    virtual void Tick(float deltaTime) override;

    // Event & Input-Driven Camera Effects
    void TriggerShake(float intensity = 0.5f, float duration = 0.3f);
    void Deflect(const TEVector2 &deflectionVector, float recoverySpeed = 10.0f);

    TEVector2 GetCalculatedCameraPosition() const;
    float GetCurrentShakeOffsetMagnitude() const;

    inline static const TEString StaticClassName = "PlayerCameraComponent";
    virtual TEString GetClassName() const override { return StaticClassName; }

private:
    float m_ShakeIntensity = 0.0f;
    float m_ShakeTimer = 0.0f;
    float m_ShakeDuration = 0.0f;

    TEVector2 m_DeflectionOffset = {0.0f, 0.0f};
    float m_DeflectionRecoverySpeed = 10.0f;

    TEVector2 m_CurrentCameraPosition = {0.0f, 0.0f};
};

#ifdef TE_EDITOR
T_REGISTER_COMPONENT(PlayerCameraComponent, "Player Dedicated Camera Component")
T_REGISTER_PROPERTY(PlayerCameraComponent, bool, bFollowOwner, "Follow Owner")
T_REGISTER_PROPERTY(PlayerCameraComponent, float, SmoothSpeed, "Smooth Speed")
T_REGISTER_PROPERTY(PlayerCameraComponent, TEVector2, FollowOffset, "Follow Offset")
T_REGISTER_PROPERTY(PlayerCameraComponent, float, OrthographicSize, "Orthographic Size")
T_REGISTER_PRESET(PlayerCamera, "Player Dedicated Camera", "Camera",
                  [](EntityID id, EntityManager *em) { em->AddComponent<PlayerCameraComponent>(id); })
#endif
