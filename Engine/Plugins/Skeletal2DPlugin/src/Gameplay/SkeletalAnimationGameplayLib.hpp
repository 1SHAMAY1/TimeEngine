#pragma once

#include "Core/PreRequisites.h"
#include "GameFrameWork/TFunctionLibrary.hpp"
#include "Core/Scene/EntityManager.hpp"
#include "Utils/TEString.hpp"
#include <glm/glm.hpp>

class SkeletalAnimationGameplayLib : public TFunctionLibrary
{
public:
    inline static const TEString StaticClassName = "SkeletalAnimationGameplayLib";

    static bool PlayAnimation(Entity entity, const TEString& animName, bool loop = true, float crossfade = 0.2f, int track = 0);
    static void StopAnimation(Entity entity);
    static void SetTimeScale(Entity entity, float timeScale);
    static bool GetBoneTransform(Entity entity, const TEString& boneName, glm::mat4& outTransform);
    static bool GetBonePosition(Entity entity, const TEString& boneName, glm::vec2& outPosition);
    static bool AttachEntityToBone(Entity skeletalEntity, Entity childEntity, const TEString& boneName, const glm::vec2& offset = {0.0f, 0.0f});
};
