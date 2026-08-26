#include "Core/PreRequisites.h"
#include "Core/Scene/PlayerCameraComponent.hpp"
#include "Core/Scene/EntityManager.hpp"
#include "Core/Scene/TransformComponent.hpp"

#include <cmath>
#include <cstdlib>

PlayerCameraComponent::PlayerCameraComponent() {}

void PlayerCameraComponent::OnAttach()
{
    Entity ownerEntity = GetOwnerEntity();
    if (ownerEntity.IsValid())
    {
        auto *transform = ownerEntity.GetComponent<TransformComponent>();
        if (transform)
        {
            m_CurrentCameraPosition = TEVector2(transform->Transform.Position.x, transform->Transform.Position.y);
        }
    }
}

void PlayerCameraComponent::TriggerShake(float intensity, float duration)
{
    m_ShakeIntensity = intensity;
    m_ShakeDuration = duration;
    m_ShakeTimer = duration;
}

void PlayerCameraComponent::Deflect(const TEVector2 &deflectionVector, float recoverySpeed)
{
    m_DeflectionOffset.x += deflectionVector.x;
    m_DeflectionOffset.y += deflectionVector.y;
    m_DeflectionRecoverySpeed = recoverySpeed;
}

void PlayerCameraComponent::Tick(float deltaTime)
{
    if (deltaTime <= 0.0f)
        return;

    // 1. Follow Owner Target
    TEVector2 targetPos = FollowOffset;
    Entity ownerEntity = GetOwnerEntity();
    if (bFollowOwner && ownerEntity.IsValid())
    {
        auto *transform = ownerEntity.GetComponent<TransformComponent>();
        if (transform)
        {
            targetPos.x += transform->Transform.Position.x;
            targetPos.y += transform->Transform.Position.y;
        }
    }

    // Smooth Interpolation
    m_CurrentCameraPosition.x += (targetPos.x - m_CurrentCameraPosition.x) * std::min(1.0f, SmoothSpeed * deltaTime);
    m_CurrentCameraPosition.y += (targetPos.y - m_CurrentCameraPosition.y) * std::min(1.0f, SmoothSpeed * deltaTime);

    // 2. Camera Shake Decay
    if (m_ShakeTimer > 0.0f)
    {
        m_ShakeTimer -= deltaTime;
        if (m_ShakeTimer <= 0.0f)
        {
            m_ShakeIntensity = 0.0f;
            m_ShakeTimer = 0.0f;
        }
    }

    // 3. Deflection Offset Recovery
    if (m_DeflectionOffset.LengthSquared() > 0.0001f)
    {
        m_DeflectionOffset.x -= m_DeflectionOffset.x * std::min(1.0f, m_DeflectionRecoverySpeed * deltaTime);
        m_DeflectionOffset.y -= m_DeflectionOffset.y * std::min(1.0f, m_DeflectionRecoverySpeed * deltaTime);
    }
}

TEVector2 PlayerCameraComponent::GetCalculatedCameraPosition() const
{
    TEVector2 finalPos = m_CurrentCameraPosition + m_DeflectionOffset;

    if (m_ShakeTimer > 0.0f && m_ShakeIntensity > 0.0f)
    {
        float shakeProgress = m_ShakeTimer / (m_ShakeDuration > 0.0f ? m_ShakeDuration : 1.0f);
        float currentIntensity = m_ShakeIntensity * shakeProgress;

        float rx = ((float)rand() / (float)RAND_MAX * 2.0f - 1.0f) * currentIntensity;
        float ry = ((float)rand() / (float)RAND_MAX * 2.0f - 1.0f) * currentIntensity;

        finalPos.x += rx;
        finalPos.y += ry;
    }

    return finalPos;
}

float PlayerCameraComponent::GetCurrentShakeOffsetMagnitude() const
{
    return (m_ShakeTimer > 0.0f) ? m_ShakeIntensity : 0.0f;
}
