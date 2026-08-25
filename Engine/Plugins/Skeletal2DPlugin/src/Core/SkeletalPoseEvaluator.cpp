#include "SkeletalPoseEvaluator.hpp"
#include <algorithm>

namespace Skeletal2D {

SkeletalPoseEvaluator::SkeletalPoseEvaluator()
{
}

void SkeletalPoseEvaluator::SetHierarchy(const BoneHierarchy& hierarchy)
{
    m_Hierarchy = hierarchy;
    m_Hierarchy.CalculateBindPoseMatrices();
}

void SkeletalPoseEvaluator::SetSkinData(const SkinData& skinData)
{
    m_ActiveSkin = skinData;
}

void SkeletalPoseEvaluator::SetAnimation(int trackIndex, TERef<AnimationClip> clip, bool loop, float crossfadeDuration)
{
    if (!clip)
    {
        ClearTrack(trackIndex);
        return;
    }

    auto& entry = m_Tracks[trackIndex];
    entry.TrackIndex = trackIndex;

    if (entry.CurrentClip && crossfadeDuration > 0.0f && entry.IsPlaying)
    {
        entry.PreviousClip = entry.CurrentClip;
        entry.MixTime = 0.0f;
        entry.MixDuration = crossfadeDuration;
    }
    else
    {
        entry.PreviousClip = nullptr;
        entry.MixDuration = 0.0f;
    }

    entry.CurrentClip = clip;
    entry.TrackTime = 0.0f;
    entry.Loop = loop;
    entry.IsPlaying = true;
}

void SkeletalPoseEvaluator::AddAnimation(int trackIndex, TERef<AnimationClip> clip, bool loop, float delay)
{
    SetAnimation(trackIndex, clip, loop, delay);
}

void SkeletalPoseEvaluator::ClearTrack(int trackIndex)
{
    auto it = m_Tracks.find(trackIndex);
    if (it != m_Tracks.end())
    {
        it->second.IsPlaying = false;
        it->second.CurrentClip = nullptr;
        it->second.PreviousClip = nullptr;
    }
}

void SkeletalPoseEvaluator::ClearTracks()
{
    m_Tracks.clear();
}

TrackEntry* SkeletalPoseEvaluator::GetTrack(int trackIndex)
{
    auto it = m_Tracks.find(trackIndex);
    if (it != m_Tracks.end())
        return &it->second;
    return nullptr;
}

void SkeletalPoseEvaluator::Update(float dt, const glm::mat4& rootTransform)
{
    float scaledDt = dt * m_GlobalTimeScale;
    UpdateTracks(scaledDt);
    EvaluateBonePoses();
    m_Hierarchy.UpdateWorldMatrices(rootTransform);
    EvaluateDeformedMeshes();
}

void SkeletalPoseEvaluator::UpdateTracks(float dt)
{
    for (auto& pair : m_Tracks)
    {
        auto& entry = pair.second;
        if (!entry.IsPlaying || !entry.CurrentClip)
            continue;

        float effectiveDt = dt * entry.TimeScale;
        entry.TrackTime += effectiveDt;

        if (entry.CurrentClip->Duration > 0.0f)
        {
            if (entry.Loop)
            {
                while (entry.TrackTime >= entry.CurrentClip->Duration)
                    entry.TrackTime -= entry.CurrentClip->Duration;
            }
            else
            {
                if (entry.TrackTime >= entry.CurrentClip->Duration)
                {
                    entry.TrackTime = entry.CurrentClip->Duration;
                    entry.IsPlaying = false;
                }
            }
        }

        // Crossfade update
        if (entry.PreviousClip && entry.MixDuration > 0.0f)
        {
            entry.MixTime += effectiveDt;
            if (entry.MixTime >= entry.MixDuration)
            {
                entry.PreviousClip = nullptr;
                entry.MixDuration = 0.0f;
            }
        }

        // Fire events
        if (m_EventCallback && entry.CurrentClip)
        {
            for (const auto& ev : entry.CurrentClip->EventKeys)
            {
                if (entry.TrackTime >= ev.Time && (entry.TrackTime - effectiveDt) < ev.Time)
                {
                    m_EventCallback(ev);
                }
            }
        }
    }
}

void SkeletalPoseEvaluator::EvaluateBonePoses()
{
    m_Hierarchy.ResetToRestPose();

    for (const auto& pair : m_Tracks)
    {
        const auto& entry = pair.second;
        if (!entry.CurrentClip)
            continue;

        float mixAlpha = 1.0f;
        if (entry.PreviousClip && entry.MixDuration > 0.0f)
        {
            mixAlpha = std::clamp(entry.MixTime / entry.MixDuration, 0.0f, 1.0f);
        }

        for (const auto& timeline : entry.CurrentClip->BoneTimelines)
        {
            auto* bone = m_Hierarchy.GetBone(timeline.BoneName);
            if (!bone)
                continue;

            glm::vec2 currPos, currScale;
            float currRot;
            timeline.Evaluate(entry.TrackTime, currPos, currRot, currScale,
                              bone->RestPose.Position, bone->RestPose.Rotation, bone->RestPose.Scale);

            if (entry.PreviousClip && mixAlpha < 1.0f)
            {
                // Find matching timeline in previous clip
                for (const auto& prevTimeline : entry.PreviousClip->BoneTimelines)
                {
                    if (prevTimeline.BoneName == timeline.BoneName)
                    {
                        glm::vec2 prevPos, prevScale;
                        float prevRot;
                        prevTimeline.Evaluate(entry.TrackTime, prevPos, prevRot, prevScale,
                                              bone->RestPose.Position, bone->RestPose.Rotation, bone->RestPose.Scale);

                        currPos = glm::mix(prevPos, currPos, mixAlpha);
                        currRot = glm::mix(prevRot, currRot, mixAlpha);
                        currScale = glm::mix(prevScale, currScale, mixAlpha);
                        break;
                    }
                }
            }

            bone->LocalPose.Position = currPos;
            bone->LocalPose.Rotation = currRot;
            bone->LocalPose.Scale = currScale;
        }
    }
}

bool SkeletalPoseEvaluator::GetBoneWorldTransform(const TEString& boneName, glm::mat4& outMatrix) const
{
    int index = m_Hierarchy.FindBoneIndex(boneName);
    if (index >= 0)
    {
        const auto* bone = m_Hierarchy.GetBone(index);
        if (bone)
        {
            outMatrix = bone->WorldMatrix;
            return true;
        }
    }
    return false;
}

bool SkeletalPoseEvaluator::GetBoneWorldPosition(const TEString& boneName, glm::vec2& outPosition) const
{
    glm::mat4 mat;
    if (GetBoneWorldTransform(boneName, mat))
    {
        outPosition = glm::vec2(mat[3].x, mat[3].y);
        return true;
    }
    return false;
}

void SkeletalPoseEvaluator::EvaluateDeformedMeshes()
{
    for (auto& slotPair : m_ActiveSkin.Slots)
    {
        auto& slot = slotPair.second;
        if (slot.ActiveAttachmentName.IsEmpty())
            continue;

        auto attachIt = slot.Attachments.find(slot.ActiveAttachmentName.c_str());
        if (attachIt == slot.Attachments.end())
            continue;

        auto& attachment = attachIt->second;
        if (attachment.Type != AttachmentType::Mesh)
            continue;

        for (auto& vertex : attachment.Vertices)
        {
            glm::vec2 deformed = {0.0f, 0.0f};

            if (vertex.Weights.empty())
            {
                // Unweighted, follow slot bone
                const auto* bone = m_Hierarchy.GetBone(slot.BoneIndex);
                if (bone)
                {
                    glm::vec4 worldPos = bone->WorldMatrix * glm::vec4(vertex.RestPosition.x, vertex.RestPosition.y, 0.0f, 1.0f);
                    deformed = glm::vec2(worldPos.x, worldPos.y);
                }
                else
                {
                    deformed = vertex.RestPosition;
                }
            }
            else
            {
                for (const auto& weight : vertex.Weights)
                {
                    const auto* bone = m_Hierarchy.GetBone(weight.BoneIndex);
                    if (bone)
                    {
                        glm::vec4 worldPos = bone->WorldMatrix * glm::vec4(weight.Offset.x, weight.Offset.y, 0.0f, 1.0f);
                        deformed += glm::vec2(worldPos.x, worldPos.y) * weight.Weight;
                    }
                }
            }

            vertex.DeformedPosition = deformed;
        }
    }
}

} // namespace Skeletal2D
