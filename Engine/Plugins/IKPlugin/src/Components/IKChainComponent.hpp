#pragma once

#include "../Solvers/IIKSolver.hpp"
#include "Core/PreRequisites.h"
#include "Core/Scene/ComponentRegistry.hpp"
#include "GameFrameWork/TComponent.hpp"
#include "Utils/MathUtils.hpp"

class IKChain2DComponent : public TComponent
{
public:
    GENERATED_BODY(IKChain2DComponent)

    T_PROPERTY(bool, Enabled, "Enabled", true)
    T_PROPERTY(float, Weight, "Weight", 1.0f)
    T_PROPERTY(TEString, RootBoneName, "Root Bone", "thigh_r")
    T_PROPERTY(TEString, MidBoneName, "Mid Bone", "calf_r")
    T_PROPERTY(TEString, TipBoneName, "Tip Bone", "foot_r")
    T_PROPERTY(bool, BendPositive, "Bend Positive", true)

    IKChain2DComponent() = default;
    virtual ~IKChain2DComponent() override = default;

    virtual TEString GetClassName() const override { return StaticClassName; }

    void OnUpdate(float dt);

    IK::IKSolverType SolverType = IK::IKSolverType::TwoBone2D;

    void SetTargetPosition(const TEVector2 &pos) { m_TargetPosition = pos; }
    TEVector2 GetTargetPosition() const { return m_TargetPosition; }

private:
    TEVector2 m_TargetPosition = TEVector2(0.0f, 0.0f);
};

#ifdef TE_EDITOR
T_REGISTER_COMPONENT(IKChain2DComponent, "IK Chain 2D")
T_REGISTER_PROPERTY(IKChain2DComponent, bool, Enabled, "Enabled")
T_REGISTER_PROPERTY(IKChain2DComponent, float, Weight, "Weight")
T_REGISTER_PROPERTY(IKChain2DComponent, TEString, RootBoneName, "Root Bone")
T_REGISTER_PROPERTY(IKChain2DComponent, TEString, MidBoneName, "Mid Bone")
T_REGISTER_PROPERTY(IKChain2DComponent, TEString, TipBoneName, "Tip Bone")
T_REGISTER_PROPERTY(IKChain2DComponent, bool, BendPositive, "Bend Positive")
#endif
