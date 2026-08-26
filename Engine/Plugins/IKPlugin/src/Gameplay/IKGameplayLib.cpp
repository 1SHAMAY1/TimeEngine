#include "IKGameplayLib.hpp"
#include "../Components/IKChainComponent.hpp"
#include "../Solvers/TwoBoneIKSolver2D.hpp"

bool IKGameplayLib::SetIKTargetPosition(Entity entity, const TEVector2 &targetPos)
{
    if (entity && entity.HasComponent<IKChain2DComponent>())
    {
        auto *ik = entity.GetComponent<IKChain2DComponent>();
        if (ik)
        {
            ik->SetTargetPosition(targetPos);
            return true;
        }
    }
    return false;
}

bool IKGameplayLib::SetIKWeight(Entity entity, float weight)
{
    if (entity && entity.HasComponent<IKChain2DComponent>())
    {
        auto *ik = entity.GetComponent<IKChain2DComponent>();
        if (ik)
        {
            ik->Weight = weight;
            return true;
        }
    }
    return false;
}

bool IKGameplayLib::EnableIK(Entity entity, bool enabled)
{
    if (entity && entity.HasComponent<IKChain2DComponent>())
    {
        auto *ik = entity.GetComponent<IKChain2DComponent>();
        if (ik)
        {
            ik->Enabled = enabled;
            return true;
        }
    }
    return false;
}

bool IKGameplayLib::SolveTwoBone2D(const TEVector2 &root, float lenA, float lenB, const TEVector2 &target, bool bendPos,
                                   TEVector2 &outMid, float &outAngleA, float &outAngleB)
{
    return IK::TwoBoneIKSolver2D::Solve(root, lenA, lenB, target, bendPos, outMid, outAngleA, outAngleB);
}
