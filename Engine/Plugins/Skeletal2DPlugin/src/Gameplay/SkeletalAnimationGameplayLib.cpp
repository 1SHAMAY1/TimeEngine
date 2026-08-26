#include "SkeletalAnimationGameplayLib.hpp"
#include "../Components/BoneSocket2DComponent.hpp"
#include "../Components/SkeletalMesh2DComponent.hpp"
#include "Core/Scene/TransformComponent.hpp"

bool SkeletalAnimationGameplayLib::PlayAnimation(Entity entity, const TEString &animName, bool loop, float crossfade,
                                                 int track)
{
    if (entity && entity.HasComponent<SkeletalMesh2DComponent>())
    {
        auto *comp = entity.GetComponent<SkeletalMesh2DComponent>();
        if (comp)
        {
            comp->PlayAnimation(animName, loop, crossfade, track);
            return true;
        }
    }
    return false;
}

void SkeletalAnimationGameplayLib::StopAnimation(Entity entity)
{
    if (entity && entity.HasComponent<SkeletalMesh2DComponent>())
    {
        auto *comp = entity.GetComponent<SkeletalMesh2DComponent>();
        if (comp)
            comp->Stop();
    }
}

void SkeletalAnimationGameplayLib::SetTimeScale(Entity entity, float timeScale)
{
    if (entity && entity.HasComponent<SkeletalMesh2DComponent>())
    {
        auto *comp = entity.GetComponent<SkeletalMesh2DComponent>();
        if (comp)
            comp->TimeScale = timeScale;
    }
}

bool SkeletalAnimationGameplayLib::GetBoneTransform(Entity entity, const TEString &boneName, glm::mat4 &outTransform)
{
    if (entity && entity.HasComponent<SkeletalMesh2DComponent>())
    {
        const auto *comp = entity.GetComponent<SkeletalMesh2DComponent>();
        return comp ? comp->GetBoneWorldTransform(boneName, outTransform) : false;
    }
    return false;
}

bool SkeletalAnimationGameplayLib::GetBonePosition(Entity entity, const TEString &boneName, glm::vec2 &outPosition)
{
    if (entity && entity.HasComponent<SkeletalMesh2DComponent>())
    {
        const auto *comp = entity.GetComponent<SkeletalMesh2DComponent>();
        return comp ? comp->GetBoneWorldPosition(boneName, outPosition) : false;
    }
    return false;
}

bool SkeletalAnimationGameplayLib::AttachEntityToBone(Entity skeletalEntity, Entity childEntity,
                                                      const TEString &boneName, const glm::vec2 &offset)
{
    if (!skeletalEntity || !childEntity || !skeletalEntity.HasComponent<SkeletalMesh2DComponent>())
        return false;

    auto *skeletalComp = skeletalEntity.GetComponent<SkeletalMesh2DComponent>();
    auto *socket = childEntity.GetComponent<BoneSocket2DComponent>();
    if (!socket)
        socket = childEntity.AddComponent<BoneSocket2DComponent>();

    if (socket)
    {
        socket->SetTargetSkeletalComponent(skeletalComp);
        socket->TargetBoneName = boneName;
        socket->LocalOffset = TEVector(offset.x, offset.y, 0.0f);
        return true;
    }
    return false;
}
