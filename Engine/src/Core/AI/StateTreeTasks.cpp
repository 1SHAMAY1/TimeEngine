#include "Core/AI/StateTreeTasks.hpp"
#include "Core/Log.h"
#include "Core/PreRequisites.h"
#include "Core/Scene/AnimatedSpriteComponent.hpp"
#include "Core/Scene/AudioSource2DComponent.hpp"
#include "Core/Scene/EntityManager.hpp"
#include "Core/Scene/NavigationAgent2DComponent.hpp"
#include "Core/Scene/TagComponent.hpp"

// STMoveToPositionTask
void STMoveToPositionTask::EnterState(Entity agent)
{
    if (!agent.IsValid())
        return;

    auto nav = agent.GetComponent<NavigationAgent2DComponent>();
    if (nav)
    {
        nav->SetTarget(TargetPosition);
    }
}

StateTaskStatus STMoveToPositionTask::TickState(Entity agent, float dt)
{
    if (!agent.IsValid())
        return StateTaskStatus::Failed;

    auto nav = agent.GetComponent<NavigationAgent2DComponent>();
    if (!nav)
        return StateTaskStatus::Failed;

    if (nav->HasReachedTarget())
        return StateTaskStatus::Succeeded;

    return StateTaskStatus::Running;
}

void STMoveToPositionTask::ExitState(Entity agent)
{
    if (agent.IsValid())
    {
        auto nav = agent.GetComponent<NavigationAgent2DComponent>();
        if (nav)
            nav->StopNavigation();
    }
}

// STMoveToEntityTask
void STMoveToEntityTask::EnterState(Entity agent)
{
    if (!agent.IsValid())
        return;

    auto nav = agent.GetComponent<NavigationAgent2DComponent>();
    if (nav)
    {
        if (TargetEntityID != 0)
        {
            nav->SetTargetEntity(TargetEntityID);
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
                        EntityID entId = reinterpret_cast<EntityID>(tagComp->GetOwner());
                        TargetEntityID = entId;
                        nav->SetTargetEntity(entId);
                        break;
                    }
                }
            }
        }
    }
}

StateTaskStatus STMoveToEntityTask::TickState(Entity agent, float dt)
{
    if (!agent.IsValid())
        return StateTaskStatus::Failed;

    auto nav = agent.GetComponent<NavigationAgent2DComponent>();
    if (!nav)
        return StateTaskStatus::Failed;

    if (nav->HasReachedTarget())
        return StateTaskStatus::Succeeded;

    return StateTaskStatus::Running;
}

void STMoveToEntityTask::ExitState(Entity agent)
{
    if (agent.IsValid())
    {
        auto nav = agent.GetComponent<NavigationAgent2DComponent>();
        if (nav)
            nav->StopNavigation();
    }
}

// STPlayAnimationTask
void STPlayAnimationTask::EnterState(Entity agent)
{
    if (!agent.IsValid())
        return;

    auto anim = agent.GetComponent<AnimatedSpriteComponent>();
    if (anim)
    {
        anim->Play(ClipName);
    }
}

StateTaskStatus STPlayAnimationTask::TickState(Entity agent, float dt) { return StateTaskStatus::Running; }

void STPlayAnimationTask::ExitState(Entity agent) {}

// STWaitTask
void STWaitTask::EnterState(Entity agent) { Elapsed = 0.0f; }

StateTaskStatus STWaitTask::TickState(Entity agent, float dt)
{
    Elapsed += dt;
    if (Elapsed >= Duration)
        return StateTaskStatus::Succeeded;
    return StateTaskStatus::Running;
}

void STWaitTask::ExitState(Entity agent) { Elapsed = 0.0f; }

// STLookAtTask
void STLookAtTask::EnterState(Entity agent) {}

StateTaskStatus STLookAtTask::TickState(Entity agent, float dt)
{
    if (!agent.IsValid() || TargetEntityID == 0)
        return StateTaskStatus::Failed;

    Entity target(TargetEntityID, agent.GetEntityManager());
    if (!target.IsValid())
        return StateTaskStatus::Failed;

    const auto &agentT = agent.GetTransform();
    const auto &targetT = target.GetTransform();

    float dx = targetT.Position.x - agentT.Position.x;
    float dy = targetT.Position.y - agentT.Position.y;
    float angle = std::atan2(dy, dx) * (180.0f / 3.14159265f);

    agent.GetTransform().Rotation.Yaw = angle;
    return StateTaskStatus::Running;
}

void STLookAtTask::ExitState(Entity agent) {}

// STPlaySoundTask
void STPlaySoundTask::EnterState(Entity agent)
{
    if (!agent.IsValid() || SoundClip.IsEmpty())
        return;

    auto audio = agent.GetComponent<AudioSource2DComponent>();
    if (audio)
    {
        audio->Play();
    }
}

StateTaskStatus STPlaySoundTask::TickState(Entity agent, float dt) { return StateTaskStatus::Succeeded; }

void STPlaySoundTask::ExitState(Entity agent) {}

// STExecuteTScriptTask
void STExecuteTScriptTask::EnterState(Entity agent)
{
    if (agent.IsValid())
    {
        for (auto &script : agent.GetScripts())
        {
            script.DispatchTimer("on_state_enter");
        }
    }
}

StateTaskStatus STExecuteTScriptTask::TickState(Entity agent, float dt)
{
    if (agent.IsValid())
    {
        for (auto &script : agent.GetScripts())
        {
            script.DispatchTimer(FunctionName);
        }
    }
    return StateTaskStatus::Running;
}

void STExecuteTScriptTask::ExitState(Entity agent)
{
    if (agent.IsValid())
    {
        for (auto &script : agent.GetScripts())
        {
            script.DispatchTimer("on_state_exit");
        }
    }
}

// STSetTagTask
void STSetTagTask::EnterState(Entity agent)
{
#if defined(TE_HAS_PLUGIN_GAMEPLAYTAGPLUGIN) || defined(TE_PLUGIN_GAMEPLAYTAGPLUGIN)
    if (agent.IsValid())
    {
        auto tagComp = agent.GetComponent<GameplayTagComponent>();
        if (tagComp)
        {
            if (!AddTag.IsEmpty())
                tagComp->has_tag(AddTag.c_str());
        }
    }
#endif
}

StateTaskStatus STSetTagTask::TickState(Entity agent, float dt) { return StateTaskStatus::Succeeded; }

void STSetTagTask::ExitState(Entity agent)
{
#if defined(TE_HAS_PLUGIN_GAMEPLAYTAGPLUGIN) || defined(TE_PLUGIN_GAMEPLAYTAGPLUGIN)
    if (agent.IsValid())
    {
        auto tagComp = agent.GetComponent<GameplayTagComponent>();
        if (tagComp)
        {
            // Optional remove on exit
        }
    }
#endif
}
