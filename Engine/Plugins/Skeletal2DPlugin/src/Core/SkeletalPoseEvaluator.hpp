#pragma once

#include <vector>

#include "AnimationTrack.hpp"
#include "BoneHierarchy.hpp"
#include "SkinTable.hpp"
#include <functional>
#include <unordered_map>

namespace Skeletal2D
{

class SkeletalPoseEvaluator
{
public:
    SkeletalPoseEvaluator();
    ~SkeletalPoseEvaluator() = default;

    void SetHierarchy(const BoneHierarchy &hierarchy);
    BoneHierarchy &GetHierarchy() { return m_Hierarchy; }
    const BoneHierarchy &GetHierarchy() const { return m_Hierarchy; }

    void SetSkinData(const SkinData &skinData);
    SkinData &GetSkinData() { return m_ActiveSkin; }
    const SkinData &GetSkinData() const { return m_ActiveSkin; }

    // Track Management
    void SetAnimation(int trackIndex, TERef<AnimationClip> clip, bool loop, float crossfadeDuration = 0.2f);
    void AddAnimation(int trackIndex, TERef<AnimationClip> clip, bool loop, float delay = 0.0f);
    void ClearTrack(int trackIndex);
    void ClearTracks();

    TrackEntry *GetTrack(int trackIndex);
    void SetTimeScale(float scale) { m_GlobalTimeScale = scale; }
    float GetTimeScale() const { return m_GlobalTimeScale; }

    // Update and Evaluation
    void Update(float dt, const glm::mat4 &rootTransform);

    // Bone Transform Queries
    bool GetBoneWorldTransform(const TEString &boneName, glm::mat4 &outMatrix) const;
    bool GetBoneWorldPosition(const TEString &boneName, glm::vec2 &outPosition) const;

    // Skinning / Mesh Deform
    void EvaluateDeformedMeshes();

    // Event Dispatch
    using EventCallback = std::function<void(const EventKeyframe &)>;
    void SetEventCallback(EventCallback callback) { m_EventCallback = callback; }

private:
    void UpdateTracks(float dt);
    void EvaluateBonePoses();

private:
    BoneHierarchy m_Hierarchy;
    SkinData m_ActiveSkin;
    TEMap<int, TrackEntry> m_Tracks;
    float m_GlobalTimeScale = 1.0f;
    EventCallback m_EventCallback = nullptr;
};

} // namespace Skeletal2D
