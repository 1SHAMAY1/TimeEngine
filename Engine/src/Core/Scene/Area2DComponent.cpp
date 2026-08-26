#include "Core/Scene/Area2DComponent.hpp"
#include "Core/PreRequisites.h"
#include "Core/Scene/TransformComponent.hpp"

Area2DComponent::Area2DComponent()
{
    isTrigger = true;
    isStatic = false;
    shape.type = CollisionType::AABB;
}

void Area2DComponent::OnAttach()
{
    isTrigger = true;
    isStatic = false;
}

void Area2DComponent::Tick(float deltaTime)
{
    // Per-frame monitoring updates if needed
}

void Area2DComponent::OnUpdateShape(const TEMatrix4 &worldTransform)
{
    TEVector2 pos = {worldTransform[3].x, worldTransform[3].y};
    TEVector2 halfSize = {Size.x * 0.5f * TEVector::Length(TEVector(worldTransform[0])),
                          Size.y * 0.5f * TEVector::Length(TEVector(worldTransform[1]))};
    shape.aabb.min = {pos.x - halfSize.x, pos.y - halfSize.y};
    shape.aabb.max = {pos.x + halfSize.x, pos.y + halfSize.y};
}

void Area2DComponent::UpdateOverlaps(const TEArray<EntityID> &newOverlaps)
{
    if (!MonitoringEnabled)
        return;

    TESet<EntityID> newSet;
    for (size_t i = 0; i < newOverlaps.Num(); ++i)
    {
        EntityID id = newOverlaps[i];
        if (id == GetOwnerEntity().GetID())
            continue;

        newSet.Add(id);

        if (!m_CurrentOverlaps.Contains(id))
        {
            // Entered
            if (OnAreaEntered)
                OnAreaEntered(id);
        }
    }

    for (EntityID id : m_CurrentOverlaps)
    {
        if (!newSet.Contains(id))
        {
            // Exited
            if (OnAreaExited)
                OnAreaExited(id);
        }
    }

    m_CurrentOverlaps = newSet;
}
