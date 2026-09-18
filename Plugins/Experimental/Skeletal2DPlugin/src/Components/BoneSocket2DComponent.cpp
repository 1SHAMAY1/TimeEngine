#include "BoneSocket2DComponent.hpp"
#include "EntityManager.hpp"
#include "TransformComponent.hpp"

void BoneSocket2DComponent::OnUpdate(float dt)
{
    if (!m_TargetSkeletalComp || TargetBoneName.IsEmpty())
        return;

    TEMatrix4 boneTransform;
    if (m_TargetSkeletalComp->GetBoneWorldTransform(TargetBoneName, boneTransform))
    {
        Entity owner = GetOwnerEntity();
        if (owner && owner.HasComponent<TransformComponent>())
        {
            auto *tc = owner.GetComponent<TransformComponent>();
            if (tc)
            {
                TEVector4 localPos(LocalOffset.x, LocalOffset.y, 0.0f, 1.0f);
                TEVector4 worldPos = boneTransform * localPos;

                tc->Transform.Position.x = worldPos.x;
                tc->Transform.Position.y = worldPos.y;
                tc->Transform.Position.z = LocalOffset.z;

                // Extract 2D rotation
                float boneRot = std::atan2(boneTransform.m[0][1], boneTransform.m[0][0]);
                tc->Transform.Rotation.Roll = boneRot + RotationOffset;
            }
        }
    }
}
