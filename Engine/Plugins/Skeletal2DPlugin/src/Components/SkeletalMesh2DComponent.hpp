#pragma once

#include "Core/PreRequisites.h"
#include "Core/Scene/ComponentRegistry.hpp"
#include "GameFrameWork/TComponent.hpp"
#include "Renderer/TEColor.hpp"
#include "Utils/MathUtils.hpp"
#include "../Assets/SkeletalDataAsset.hpp"
#include "../Core/SkeletalPoseEvaluator.hpp"

class SkeletalMesh2DComponent : public TComponent
{
public:
    GENERATED_BODY(SkeletalMesh2DComponent)

    T_PROPERTY(bool, Playing, "Playing", true)
    T_PROPERTY(bool, Loop, "Loop", true)
    T_PROPERTY(float, TimeScale, "Time Scale", 1.0f)
    T_PROPERTY(TEString, DefaultAnimation, "Default Animation", "idle")
    T_PROPERTY(TEString, ActiveSkin, "Skin", "default")
    T_PROPERTY(TEColor, TintColor, "Tint Color", TEColor::White())
    T_PROPERTY(bool, ShowBoneGizmos, "Show Bone Gizmos", false)

    SkeletalMesh2DComponent();
    virtual ~SkeletalMesh2DComponent() = default;

    void OnCreate();
    void OnUpdate(float dt);
    void OnRender(class Renderer2D* renderer, const glm::mat4& transform, void* userPayload = nullptr);

    // Skeletal Asset Binding
    void SetSkeletalAsset(TERef<Skeletal2D::SkeletalDataAsset> asset);
    TERef<Skeletal2D::SkeletalDataAsset> GetSkeletalAsset() const { return m_SkeletalAsset; }

    // Playback Controls
    void PlayAnimation(const TEString& name, bool loop = true, float crossfade = 0.2f, int track = 0);
    void Pause() { Playing = false; }
    void Resume() { Playing = true; }
    void Stop();

    // Bone Transform Queries
    bool GetBoneWorldTransform(const TEString& boneName, glm::mat4& outTransform) const;
    bool GetBoneWorldPosition(const TEString& boneName, glm::vec2& outPos) const;

    Skeletal2D::SkeletalPoseEvaluator& GetEvaluator() { return m_Evaluator; }
    const Skeletal2D::SkeletalPoseEvaluator& GetEvaluator() const { return m_Evaluator; }

private:
    TERef<Skeletal2D::SkeletalDataAsset> m_SkeletalAsset = nullptr;
    Skeletal2D::SkeletalPoseEvaluator m_Evaluator;
    glm::mat4 m_CachedWorldTransform = glm::mat4(1.0f);
};
