#pragma once

#include "Core/PreRequisites.h"
#include "Core/Scene/EntityManager.hpp"
#include "GameFrameWork/TFunctionLibrary.hpp"
#include "Utils/MathUtils.hpp"
#include "Utils/TEString.hpp"

class SkeletalAnimationGameplayLib : public TFunctionLibrary
{
public:
    inline static const TEString StaticClassName = "SkeletalAnimationGameplayLib";

    static bool PlayAnimation(Entity entity, const TEString &animName, bool loop = true, float crossfade = 0.2f,
                              int track = 0);
    static void StopAnimation(Entity entity);
    static void SetTimeScale(Entity entity, float timeScale);
    static bool GetBoneTransform(Entity entity, const TEString &boneName, TEMatrix4 &outTransform);
    static bool GetBonePosition(Entity entity, const TEString &boneName, TEVector2 &outPosition);
    static bool AttachEntityToBone(Entity skeletalEntity, Entity childEntity, const TEString &boneName,
                                   const TEVector2 &offset = {0.0f, 0.0f});
};
