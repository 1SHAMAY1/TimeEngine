#include "SkeletalMesh2DComponent.hpp"
#include "Core/Log.h"
#include "Renderer/Renderer2D.hpp"

SkeletalMesh2DComponent::SkeletalMesh2DComponent() {}

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

void SkeletalMesh2DComponent::OnRender(Renderer2D *renderer, const TEMatrix4 &transform, void *userPayload)
{
    if (!renderer)
        return;

    m_CachedWorldTransform = transform;

    // Render deformed skin slots and attachments
    const auto &skin = m_Evaluator.GetSkinData();
    for (const auto &slotPair : skin.Slots)
    {
        const auto &slot = slotPair.second;
        if (slot.ActiveAttachmentName.IsEmpty())
            continue;

        auto attachIt = slot.Attachments.find(slot.ActiveAttachmentName);
        if (attachIt == slot.Attachments.end())
            continue;

        const auto &attachment = attachIt->second;
        if (attachment.Type == Skeletal2D::AttachmentType::Mesh)
        {
            for (size_t i = 0; i + 2 < attachment.Indices.size(); i += 3)
            {
                uint32_t i0 = attachment.Indices[i];
                uint32_t i1 = attachment.Indices[i + 1];
                uint32_t i2 = attachment.Indices[i + 2];

                if (i0 < attachment.Vertices.size() && i1 < attachment.Vertices.size() &&
                    i2 < attachment.Vertices.size())
                {
                    TEVector2 p0 = attachment.Vertices[i0].DeformedPosition;
                    TEVector2 p1 = attachment.Vertices[i1].DeformedPosition;
                    TEVector2 p2 = attachment.Vertices[i2].DeformedPosition;

                    renderer->SubmitTriangle(p0, p1, p2, nullptr);
                }
            }
        }
    }

    // Debug Bone Gizmos
    if (ShowBoneGizmos)
    {
        const auto &bones = m_Evaluator.GetHierarchy().GetBones();
        for (const auto &bone : bones)
        {
            TEVector2 startPos = TEVector2(bone.WorldMatrix.m[3][0], bone.WorldMatrix.m[3][1]);
            TEVector4 tipLocal = TEVector4(bone.Length, 0.0f, 0.0f, 1.0f);
            TEVector4 tipWorld = bone.WorldMatrix * tipLocal;
            TEVector2 endPos = TEVector2(tipWorld.x, tipWorld.y);

            renderer->SubmitLine(startPos, endPos, 2.0f, TEColor(0.2f, 0.8f, 1.0f, 0.8f));
            renderer->SubmitCircle(startPos, 3.0f, nullptr);
        }
    }
}

void SkeletalMesh2DComponent::PlayAnimation(const TEString &name, bool loop, float crossfade, int track)
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

bool SkeletalMesh2DComponent::GetBoneWorldTransform(const TEString &boneName, TEMatrix4 &outTransform) const
{
    return m_Evaluator.GetBoneWorldTransform(boneName, outTransform);
}

bool SkeletalMesh2DComponent::GetBoneWorldPosition(const TEString &boneName, TEVector2 &outPos) const
{
    return m_Evaluator.GetBoneWorldPosition(boneName, outPos);
}
