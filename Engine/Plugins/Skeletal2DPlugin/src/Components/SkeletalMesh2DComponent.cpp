#include "SkeletalMesh2DComponent.hpp"
#include "Renderer/Renderer2D.hpp"
#include "Core/Log.h"

SkeletalMesh2DComponent::SkeletalMesh2DComponent()
{
}

void SkeletalMesh2DComponent::OnCreate()
{
    if (m_SkeletalAsset)
    {
        SetSkeletalAsset(m_SkeletalAsset);
    }
}

void SkeletalMesh2DComponent::SetSkeletalAsset(TERef<Skeletal2D::SkeletalDataAsset> asset)
{
    m_SkeletalAsset = asset;
    if (m_SkeletalAsset)
    {
        m_Evaluator.SetHierarchy(m_SkeletalAsset->GetHierarchy());
        m_Evaluator.SetSkinData(m_SkeletalAsset->GetDefaultSkin());

        if (!DefaultAnimation.IsEmpty())
        {
            auto clip = m_SkeletalAsset->GetAnimation(DefaultAnimation);
            if (clip)
            {
                m_Evaluator.SetAnimation(0, clip, Loop, 0.0f);
            }
        }
    }
}

void SkeletalMesh2DComponent::OnUpdate(float dt)
{
    if (!Playing || !m_SkeletalAsset)
        return;

    m_Evaluator.SetTimeScale(TimeScale);
    m_Evaluator.Update(dt, m_CachedWorldTransform);
}

void SkeletalMesh2DComponent::OnRender(Renderer2D* renderer, const glm::mat4& transform, void* userPayload)
{
    if (!renderer)
        return;

    m_CachedWorldTransform = transform;

    // Render deformed skin slots and attachments
    const auto& skin = m_Evaluator.GetSkinData();
    for (const auto& slotPair : skin.Slots)
    {
        const auto& slot = slotPair.second;
        if (slot.ActiveAttachmentName.IsEmpty())
            continue;

        auto attachIt = slot.Attachments.find(slot.ActiveAttachmentName.c_str());
        if (attachIt == slot.Attachments.end())
            continue;

        const auto& attachment = attachIt->second;
        if (attachment.Type == Skeletal2D::AttachmentType::Mesh)
        {
            for (size_t i = 0; i + 2 < attachment.Indices.size(); i += 3)
            {
                uint32_t i0 = attachment.Indices[i];
                uint32_t i1 = attachment.Indices[i + 1];
                uint32_t i2 = attachment.Indices[i + 2];

                if (i0 < attachment.Vertices.size() &&
                    i1 < attachment.Vertices.size() &&
                    i2 < attachment.Vertices.size())
                {
                    glm::vec2 p0 = attachment.Vertices[i0].DeformedPosition;
                    glm::vec2 p1 = attachment.Vertices[i1].DeformedPosition;
                    glm::vec2 p2 = attachment.Vertices[i2].DeformedPosition;

                    renderer->SubmitTriangle(TEVector2(p0.x, p0.y), TEVector2(p1.x, p1.y), TEVector2(p2.x, p2.y), nullptr);
                }
            }
        }
    }

    // Debug Bone Gizmos
    if (ShowBoneGizmos)
    {
        const auto& bones = m_Evaluator.GetHierarchy().GetBones();
        for (const auto& bone : bones)
        {
            glm::vec2 startPos = glm::vec2(bone.WorldMatrix[3].x, bone.WorldMatrix[3].y);
            glm::vec4 tipLocal = glm::vec4(bone.Length, 0.0f, 0.0f, 1.0f);
            glm::vec4 tipWorld = bone.WorldMatrix * tipLocal;
            glm::vec2 endPos = glm::vec2(tipWorld.x, tipWorld.y);

            renderer->SubmitLine(TEVector2(startPos.x, startPos.y), TEVector2(endPos.x, endPos.y), 2.0f, TEColor(0.2f, 0.8f, 1.0f, 0.8f));
            renderer->SubmitCircle(TEVector2(startPos.x, startPos.y), 3.0f, nullptr);
        }
    }
}

void SkeletalMesh2DComponent::PlayAnimation(const TEString& name, bool loop, float crossfade, int track)
{
    if (!m_SkeletalAsset)
        return;

    auto clip = m_SkeletalAsset->GetAnimation(name);
    if (clip)
    {
        Playing = true;
        Loop = loop;
        m_Evaluator.SetAnimation(track, clip, loop, crossfade);
    }
}

void SkeletalMesh2DComponent::Stop()
{
    Playing = false;
    m_Evaluator.ClearTracks();
}

bool SkeletalMesh2DComponent::GetBoneWorldTransform(const TEString& boneName, glm::mat4& outTransform) const
{
    return m_Evaluator.GetBoneWorldTransform(boneName, outTransform);
}

bool SkeletalMesh2DComponent::GetBoneWorldPosition(const TEString& boneName, glm::vec2& outPos) const
{
    return m_Evaluator.GetBoneWorldPosition(boneName, outPos);
}
