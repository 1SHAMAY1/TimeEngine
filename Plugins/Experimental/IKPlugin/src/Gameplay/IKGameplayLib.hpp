#pragma once

#include "PreRequisites.h"
#include "EntityManager.hpp"
#include "TFunctionLibrary.hpp"
#include "MathUtils.hpp"
#include "EngineTypes/TEString.hpp"

class IKGameplayLib : public TFunctionLibrary
{
public:
    inline static const TEString StaticClassName = "IKGameplayLib";

    static bool SetIKTargetPosition(Entity entity, const TEVector2 &targetPos);
    static bool SetIKWeight(Entity entity, float weight);
    static bool EnableIK(Entity entity, bool enabled);
    static bool SolveTwoBone2D(const TEVector2 &root, float lenA, float lenB, const TEVector2 &target, bool bendPos,
                               TEVector2 &outMid, float &outAngleA, float &outAngleB);
};
