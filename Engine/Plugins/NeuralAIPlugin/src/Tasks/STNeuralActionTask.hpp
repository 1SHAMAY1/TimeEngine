#pragma once

#include "Core/PreRequisites.h"
#include "Core/AI/StateTreeTasks.hpp"

#if defined(TE_HAS_PLUGIN_GAMEPLAYTAGPLUGIN) || defined(TE_PLUGIN_GAMEPLAYTAGPLUGIN)
#include "../../../../Plugins/GameplayTagPlugin/src/GameplayTagContainer.hpp"
#endif

class STNeuralActionTask : public IStateTask
{
public:
    TEString ModelAssetPath = "";
    int ActionCount = 4;

    virtual void EnterState(Entity agent) override;
    virtual StateTaskStatus TickState(Entity agent, float dt) override;
    virtual void ExitState(Entity agent) override;
    virtual TEString GetTaskType() const override { return "STNeuralActionTask"; }
    virtual TERef<IStateTask> Clone() const override { return CreateRef<STNeuralActionTask>(*this); }
};
