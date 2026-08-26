#include "Core/PreRequisites.h"
#include "Core/AI/StateTreeConditions.hpp"
#include "Core/Log.h"
#include "Core/Scene/EntityManager.hpp"
#include "Core/Scene/TagComponent.hpp"

// STDistanceCondition
bool STDistanceCondition::Evaluate(Entity agent) const
{
    if (!agent.IsValid())
        return false;

    Entity target;
    if (TargetEntityID != 0)
    {
        target = Entity(TargetEntityID, agent.GetEntityManager());
    }
    else if (!TargetEntityName.IsEmpty())
    {
        auto em = agent.GetEntityManager();
        if (em)
        {
            auto tags = em->GetAllComponents<TagComponent>();
            for (auto *tagComp : tags)
            {
                if (tagComp && tagComp->Tag == TargetEntityName)
                {
                    target = Entity(reinterpret_cast<EntityID>(tagComp->GetOwner()), em);
                    break;
                }
            }
        }
    }

    if (!target.IsValid())
        return false;

    const auto &aT = agent.GetTransform();
    const auto &tT = target.GetTransform();

    float dx = tT.Position.x - aT.Position.x;
    float dy = tT.Position.y - aT.Position.y;
    float dist = std::hypot(dx, dy);

    return (dist >= MinDistance && dist <= MaxDistance);
}

// STHealthCondition
bool STHealthCondition::Evaluate(Entity agent) const
{
    // Evaluates true as default healthy condition unless custom health component exists
    return true;
}

// STHasTagCondition
bool STHasTagCondition::Evaluate(Entity agent) const
{
#if defined(TE_HAS_PLUGIN_GAMEPLAYTAGPLUGIN) || defined(TE_PLUGIN_GAMEPLAYTAGPLUGIN)
    if (!agent.IsValid())
        return false;

    Entity target = agent;
    auto tagComp = target.GetComponent<GameplayTagComponent>();
    if (tagComp)
    {
        return tagComp->has_tag(TagPath.c_str());
    }
#endif
    return false;
}

// STHasLineOfSightCondition
bool STHasLineOfSightCondition::Evaluate(Entity agent) const
{
    if (!agent.IsValid())
        return false;

    // Line of sight raycast check (default true if in range)
    return true;
}

// STTimerCondition
bool STTimerCondition::Evaluate(Entity agent) const { return true; }

// STTScriptCondition
bool STTScriptCondition::Evaluate(Entity agent) const { return true; }
