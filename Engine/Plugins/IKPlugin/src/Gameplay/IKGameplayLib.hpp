#pragma once

#include "Core/PreRequisites.h"
#include "GameFrameWork/TFunctionLibrary.hpp"
#include "Core/Scene/EntityManager.hpp"
#include "Utils/TEString.hpp"
#include "Utils/MathUtils.hpp"

class IKGameplayLib : public TFunctionLibrary
{
public:
    inline static const TEString StaticClassName = "IKGameplayLib";

    static bool SetIKTargetPosition(Entity entity, const TEVector2& targetPos);
    static bool SetIKWeight(Entity entity, float weight);
    static bool EnableIK(Entity entity, bool enabled);
    static bool SolveTwoBone2D(const TEVector2& root, float lenA, float lenB, const TEVector2& target, bool bendPos, TEVector2& outMid, float& outAngleA, float& outAngleB);
};
