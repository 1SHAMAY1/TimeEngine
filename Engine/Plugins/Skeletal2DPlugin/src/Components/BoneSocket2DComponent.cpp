#include "BoneSocket2DComponent.hpp"
#include "Core/Scene/TransformComponent.hpp"
#include "Core/Scene/EntityManager.hpp"

void BoneSocket2DComponent::OnUpdate(float dt)
{
    if (!m_TargetSkeletalComp || TargetBoneName.IsEmpty())
        return;

    glm::mat4 boneTransform;
    if (m_TargetSkeletalComp->GetBoneWorldTransform(TargetBoneName, boneTransform))
    {
        Entity owner = GetOwnerEntity();
        if (owner && owner.HasComponent<TransformComponent>())
        {
            auto* tc = owner.GetComponent<TransformComponent>();
            if (tc)
            {
                glm::vec4 localPos(LocalOffset.x, LocalOffset.y, 0.0f, 1.0f);
                glm::vec4 worldPos = boneTransform * localPos;

                tc->Transform.Position.x = worldPos.x;
                tc->Transform.Position.y = worldPos.y;
                tc->Transform.Position.z = LocalOffset.z;

                // Extract 2D rotation
                float boneRot = std::atan2(boneTransform[0][1], boneTransform[0][0]);
                tc->Transform.Rotation.Roll = boneRot + RotationOffset;
            }
        }
    }
}
