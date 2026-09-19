#include "PreRequisites.h"
#include "SweepLine2DComponent.hpp"
#include "GameplayUtils.hpp"
#include "Renderer2D.hpp"
#include "TransformComponent.hpp"

void SweepLine2DComponent::Tick(float deltaTime)
{
    // Runtime tick logic
}

bool SweepLine2DComponent::PerformSweep(Scene *scene)
{
    if (!Enabled || !scene)
    {
        HitResult = TESpatialHitResult{};
        return false;
    }

    auto *transform = GetOwnerEntity().GetComponent<TransformComponent>();
    if (!transform)
    {
        HitResult = TESpatialHitResult{};
        return false;
    }

    TEVector2 start = {transform->Transform.Position.x, transform->Transform.Position.y};
    TEVector2 dir = Direction;
    float len = TEVector2::Length(dir);
    if (len > 0.0001f)
        dir = dir / len;
    else
        dir = {0.0f, 1.0f};

    TEVector2 end = start + dir * Length;
    HitResult = GameplayUtils::SweepLine(start, end, scene, DrawDebug, 0.0f);
    return HitResult.Hit;
}

void SweepLine2DComponent::OnRender(Renderer2D *renderer, const TEMatrix4 &worldModel,
                                    const TERef<Material> &material) const
{
    if (!renderer || !Enabled)
        return;

    TEVector4 start4 = worldModel * TEVector4(0.0f, 0.0f, 0.0f, 1.0f);
    TEVector2 start(start4.x, start4.y);

    TEVector2 dir = Direction;
    float len = TEVector2::Length(dir);
    if (len > 0.0001f)
        dir = dir / len;
    else
        dir = {0.0f, 1.0f};

    TEVector2 end = HitResult.Hit ? HitResult.Point : (start + dir * Length);
    TEColor rayColor = HitResult.Hit ? TEColor(1.0f, 0.2f, 0.2f, 1.0f) : TEColor(0.2f, 1.0f, 0.2f, 0.8f);

    renderer->SubmitLine(start, end, 2.0f, rayColor);
}
