#include "IKChainComponent.hpp"
#include "../Solvers/TwoBoneIKSolver2D.hpp"
#include "Core/Scene/TransformComponent.hpp"
#include "Core/Scene/EntityManager.hpp"

void IKChain2DComponent::OnUpdate(float dt)
{
    if (!Enabled || Weight <= 0.0f)
        return;

    Entity owner = GetOwnerEntity();
    if (!owner)
        return;

    TEVector2 rootPos(0.0f, 0.0f);
    if (owner.HasComponent<TransformComponent>())
    {
        auto* tc = owner.GetComponent<TransformComponent>();
        if (tc)
            rootPos = TEVector2(tc->Transform.Position.x, tc->Transform.Position.y);
    }

    TEVector2 targetPos(m_TargetPosition.x, m_TargetPosition.y);
    TEVector2 midPos;
    float angleA, angleB;

    float lengthA = 50.0f;
    float lengthB = 50.0f;

    IK::TwoBoneIKSolver2D::Solve(rootPos, lengthA, lengthB, targetPos, BendPositive, midPos, angleA, angleB);
}
