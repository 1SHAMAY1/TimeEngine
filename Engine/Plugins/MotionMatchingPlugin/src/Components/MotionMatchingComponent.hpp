#pragma once

#include "Core/Asset/Asset.hpp"
#include "Core/Scene/ComponentRegistry.hpp"
#include "GameFrameWork/TComponent.hpp"
#include "Gameframework/GameplayUtils.hpp"

enum class MotionMatchingDimensionMode : uint8_t
{
    Sprite2D = 0,
    Skeletal2D = 1,
    Skeletal3D = 2
};

struct UnifiedPoseFeature
{
    TEVector3 Velocity = {0.0f, 0.0f, 0.0f};
    float AngularVelocity = 0.0f;
    TEVector3 TrajectoryFuture02s = {0.0f, 0.0f, 0.0f};
    TEVector3 TrajectoryFuture04s = {0.0f, 0.0f, 0.0f};
    TEVector3 TrajectoryFuture08s = {0.0f, 0.0f, 0.0f};
    float Phase = 0.0f;
};

class MotionMatchingComponent : public TComponent
{
public:
    GENERATED_BODY(MotionMatchingComponent)

    T_PROPERTY(AssetHandle, AnimationDatabase, "Animation Database", 0)
    T_PROPERTY(float, BlendTime, "Blend Time", 0.1f)

    virtual TEString GetClassName() const override { return StaticClassName; }

    MotionMatchingDimensionMode DimensionMode = MotionMatchingDimensionMode::Sprite2D;

    void OnUpdate(float dt);
    UnifiedPoseFeature BuildQueryFeature();
    int FindBestPoseIndex(const UnifiedPoseFeature &query);
};

#ifdef TE_EDITOR
T_REGISTER_COMPONENT(MotionMatchingComponent, "Motion Matching Component")
T_REGISTER_PROPERTY(MotionMatchingComponent, AssetHandle, AnimationDatabase, "Animation Database")
T_REGISTER_PROPERTY(MotionMatchingComponent, float, BlendTime, "Blend Time")
#endif
