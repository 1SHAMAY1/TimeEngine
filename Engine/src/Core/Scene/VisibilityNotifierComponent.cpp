#include "Core/Scene/VisibilityNotifierComponent.hpp"
#include "Core/PreRequisites.h"
#include "Core/Scene/EntityManager.hpp"
#include "Core/Scene/PlayerCameraComponent.hpp"
#include "Core/Scene/Scene.hpp"

void VisibilityNotifierComponent::Tick(float deltaTime)
{
    if (!Manager)
        return;

    // Viewport camera visibility check
    TEVector2 currentPos = {Transform.Position.x, Transform.Position.y};

    // Check against active camera if present
    bool onScreen = true; // default on screen if no camera bounds restrict
    auto cameras = Manager->GetAllComponents<PlayerCameraComponent>();
    if (!cameras.IsEmpty())
    {
        auto cam = cameras[0];
        if (cam)
        {
            const auto &camT = cam->Transform;
            float halfW = 640.0f;
            float halfH = 360.0f;
            if (cam->OrthographicSize > 0.001f)
            {
                halfW = cam->OrthographicSize * 100.0f;
                halfH = cam->OrthographicSize * 56.25f;
            }

            float minX = camT.Position.x - halfW - BoundsSize.x * 0.5f;
            float maxX = camT.Position.x + halfW + BoundsSize.x * 0.5f;
            float minY = camT.Position.y - halfH - BoundsSize.y * 0.5f;
            float maxY = camT.Position.y + halfH + BoundsSize.y * 0.5f;

            onScreen = (currentPos.x >= minX && currentPos.x <= maxX && currentPos.y >= minY && currentPos.y <= maxY);
        }
    }

    if (onScreen != m_bIsOnScreen)
    {
        m_bIsOnScreen = onScreen;
        if (m_bIsOnScreen)
        {
            for (auto &script : m_Scripts)
                script.DispatchTimer("on_screen_entered");
        }
        else
        {
            for (auto &script : m_Scripts)
                script.DispatchTimer("on_screen_exited");
        }
    }
}
