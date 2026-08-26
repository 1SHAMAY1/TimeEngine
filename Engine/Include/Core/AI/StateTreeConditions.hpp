#pragma once

#include "Core/PreRequisites.h"
#include "Core/Scene/Scene.hpp"
#include "GameFrameWork/GameplayUtils.hpp"
#include "Utils/TEString.hpp"
#include <memory>

#if defined(TE_HAS_PLUGIN_GAMEPLAYTAGPLUGIN) || defined(TE_PLUGIN_GAMEPLAYTAGPLUGIN)
#include "../../../Plugins/GameplayTagPlugin/src/GameplayTag.hpp"
#include "../../../Plugins/GameplayTagPlugin/src/GameplayTagComponent.hpp"
#include "../../../Plugins/GameplayTagPlugin/src/GameplayTagContainer.hpp"
#include "../../../Plugins/GameplayTagPlugin/src/GameplayTagManager.hpp"
#endif

class TE_API IStateCondition
{
public:
    virtual ~IStateCondition() = default;

    virtual bool Evaluate(Entity agent) const = 0;
    virtual TEString GetConditionType() const = 0;
    virtual TERef<IStateCondition> Clone() const = 0;
};

// 1. Distance condition
class TE_API STDistanceCondition : public IStateCondition
{
public:
    TEString TargetEntityName = "Player";
    uint64_t TargetEntityID = 0;
    float MinDistance = 0.0f;
    float MaxDistance = 1000000.0f;

    virtual bool Evaluate(Entity agent) const override;
    virtual TEString GetConditionType() const override { return "STDistanceCondition"; }
    virtual TERef<IStateCondition> Clone() const override { return CreateRef<STDistanceCondition>(*this); }
};

// 2. Health condition
class TE_API STHealthCondition : public IStateCondition
{
public:
    float MinHealth = 0.0f;
    float MaxHealth = 1000.0f;

    virtual bool Evaluate(Entity agent) const override;
    virtual TEString GetConditionType() const override { return "STHealthCondition"; }
    virtual TERef<IStateCondition> Clone() const override { return CreateRef<STHealthCondition>(*this); }
};

// 3. Has tag condition (Direct PPB guarded)
class TE_API STHasTagCondition : public IStateCondition
{
public:
    TEString TagPath = "";
    bool bTargetEntity = false; // if true checks TargetEntity, else checks self

    virtual bool Evaluate(Entity agent) const override;
    virtual TEString GetConditionType() const override { return "STHasTagCondition"; }
    virtual TERef<IStateCondition> Clone() const override { return CreateRef<STHasTagCondition>(*this); }
};

// 4. Has line of sight condition
class TE_API STHasLineOfSightCondition : public IStateCondition
{
public:
    TEString TargetEntityName = "Player";
    uint64_t TargetEntityID = 0;

    virtual bool Evaluate(Entity agent) const override;
    virtual TEString GetConditionType() const override { return "STHasLineOfSightCondition"; }
    virtual TERef<IStateCondition> Clone() const override { return CreateRef<STHasLineOfSightCondition>(*this); }
};

// 5. Timer condition (evaluated within state evaluator)
class TE_API STTimerCondition : public IStateCondition
{
public:
    float RequiredDuration = 1.0f;

    virtual bool Evaluate(Entity agent) const override;
    virtual TEString GetConditionType() const override { return "STTimerCondition"; }
    virtual TERef<IStateCondition> Clone() const override { return CreateRef<STTimerCondition>(*this); }
};

// 6. TScript condition
class TE_API STTScriptCondition : public IStateCondition
{
public:
    TEString FunctionName = "can_transition";

    virtual bool Evaluate(Entity agent) const override;
    virtual TEString GetConditionType() const override { return "STTScriptCondition"; }
    virtual TERef<IStateCondition> Clone() const override { return CreateRef<STTScriptCondition>(*this); }
};
