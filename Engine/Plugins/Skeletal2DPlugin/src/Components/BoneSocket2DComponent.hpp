#pragma once

#include "Core/PreRequisites.h"
#include "Core/Scene/ComponentRegistry.hpp"
#include "GameFrameWork/TComponent.hpp"
#include "Utils/MathUtils.hpp"
#include "SkeletalMesh2DComponent.hpp"

class BoneSocket2DComponent : public TComponent
{
public:
    GENERATED_BODY(BoneSocket2DComponent)

    T_PROPERTY(TEString, TargetBoneName, "Target Bone", "hand_r")
    T_PROPERTY(TEVector, LocalOffset, "Local Offset", TEVector(0.0f, 0.0f, 0.0f))
    T_PROPERTY(float, RotationOffset, "Rotation Offset", 0.0f)
    T_PROPERTY(bool, InheritScale, "Inherit Scale", true)

    BoneSocket2DComponent() = default;
    virtual ~BoneSocket2DComponent() = default;

    void OnUpdate(float dt);

    void SetTargetSkeletalComponent(SkeletalMesh2DComponent* skeletalComp) { m_TargetSkeletalComp = skeletalComp; }
    SkeletalMesh2DComponent* GetTargetSkeletalComponent() const { return m_TargetSkeletalComp; }

private:
    SkeletalMesh2DComponent* m_TargetSkeletalComp = nullptr;
};
