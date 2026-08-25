#pragma once

#include "Core/PreRequisites.h"
#include "Core/Scene/Scene.hpp"
#include "GameFrameWork/GameplayUtils.hpp"
#include "Utils/MathUtils.hpp"
#include "Utils/TEString.hpp"
#include <memory>

#if defined(TE_HAS_PLUGIN_GAMEPLAYTAGPLUGIN) || defined(TE_PLUGIN_GAMEPLAYTAGPLUGIN)
#include "../../Plugins/GameplayTagPlugin/src/GameplayTag.hpp"
#include "../../Plugins/GameplayTagPlugin/src/GameplayTagContainer.hpp"
#include "../../Plugins/GameplayTagPlugin/src/GameplayTagManager.hpp"
#include "../../Plugins/GameplayTagPlugin/src/GameplayTagComponent.hpp"
#endif

enum class StateTaskStatus
{
    Running,
    Succeeded,
    Failed
};

class TE_API IStateTask
{
public:
    virtual ~IStateTask() = default;

    virtual void EnterState(Entity agent) {}
    virtual StateTaskStatus TickState(Entity agent, float dt) = 0;
    virtual void ExitState(Entity agent) {}
    virtual TEString GetTaskType() const = 0;
    virtual TERef<IStateTask> Clone() const = 0;
};

// 1. Move to position task (via NavigationAgent2DComponent)
class TE_API STMoveToPositionTask : public IStateTask
{
public:
    TEVector2 TargetPosition = {0.0f, 0.0f};
    bool bPatrolMode = false;

    virtual void EnterState(Entity agent) override;
    virtual StateTaskStatus TickState(Entity agent, float dt) override;
    virtual void ExitState(Entity agent) override;
    virtual TEString GetTaskType() const override { return "STMoveToPositionTask"; }
    virtual TERef<IStateTask> Clone() const override { return CreateRef<STMoveToPositionTask>(*this); }
};

// 2. Move to entity task
class TE_API STMoveToEntityTask : public IStateTask
{
public:
    uint64_t TargetEntityID = 0;
    TEString TargetEntityName = "";

    virtual void EnterState(Entity agent) override;
    virtual StateTaskStatus TickState(Entity agent, float dt) override;
    virtual void ExitState(Entity agent) override;
    virtual TEString GetTaskType() const override { return "STMoveToEntityTask"; }
    virtual TERef<IStateTask> Clone() const override { return CreateRef<STMoveToEntityTask>(*this); }
};

// 3. Play animation task
class TE_API STPlayAnimationTask : public IStateTask
{
public:
    TEString ClipName = "idle";

    virtual void EnterState(Entity agent) override;
    virtual StateTaskStatus TickState(Entity agent, float dt) override;
    virtual void ExitState(Entity agent) override;
    virtual TEString GetTaskType() const override { return "STPlayAnimationTask"; }
    virtual TERef<IStateTask> Clone() const override { return CreateRef<STPlayAnimationTask>(*this); }
};

// 4. Wait task
class TE_API STWaitTask : public IStateTask
{
public:
    float Duration = 1.0f;
    float Elapsed = 0.0f;

    virtual void EnterState(Entity agent) override;
    virtual StateTaskStatus TickState(Entity agent, float dt) override;
    virtual void ExitState(Entity agent) override;
    virtual TEString GetTaskType() const override { return "STWaitTask"; }
    virtual TERef<IStateTask> Clone() const override { return CreateRef<STWaitTask>(*this); }
};

// 5. Look at task
class TE_API STLookAtTask : public IStateTask
{
public:
    uint64_t TargetEntityID = 0;

    virtual void EnterState(Entity agent) override;
    virtual StateTaskStatus TickState(Entity agent, float dt) override;
    virtual void ExitState(Entity agent) override;
    virtual TEString GetTaskType() const override { return "STLookAtTask"; }
    virtual TERef<IStateTask> Clone() const override { return CreateRef<STLookAtTask>(*this); }
};

// 6. Play sound task
class TE_API STPlaySoundTask : public IStateTask
{
public:
    TEString SoundClip = "";

    virtual void EnterState(Entity agent) override;
    virtual StateTaskStatus TickState(Entity agent, float dt) override;
    virtual void ExitState(Entity agent) override;
    virtual TEString GetTaskType() const override { return "STPlaySoundTask"; }
    virtual TERef<IStateTask> Clone() const override { return CreateRef<STPlaySoundTask>(*this); }
};

// 7. Execute TScript task
class TE_API STExecuteTScriptTask : public IStateTask
{
public:
    TEString ScriptPath = "";
    TEString FunctionName = "tick_task";

    virtual void EnterState(Entity agent) override;
    virtual StateTaskStatus TickState(Entity agent, float dt) override;
    virtual void ExitState(Entity agent) override;
    virtual TEString GetTaskType() const override { return "STExecuteTScriptTask"; }
    virtual TERef<IStateTask> Clone() const override { return CreateRef<STExecuteTScriptTask>(*this); }
};

// 8. Set tag task (Direct PPB guarded)
class TE_API STSetTagTask : public IStateTask
{
public:
    TEString AddTag = "";
    TEString RemoveTag = "";

    virtual void EnterState(Entity agent) override;
    virtual StateTaskStatus TickState(Entity agent, float dt) override;
    virtual void ExitState(Entity agent) override;
    virtual TEString GetTaskType() const override { return "STSetTagTask"; }
    virtual TERef<IStateTask> Clone() const override { return CreateRef<STSetTagTask>(*this); }
};
