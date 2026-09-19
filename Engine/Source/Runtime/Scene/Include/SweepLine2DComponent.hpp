#pragma once

#include "ComponentRegistry.hpp"
#include "GameplayUtils.hpp"
#include "MathUtils.hpp"
#include "TComponent.hpp"

class TE_API SweepLine2DComponent : public TComponent
{
public:
    GENERATED_BODY(SweepLine2DComponent)

    T_PROPERTY(TEVector2, Direction, "Direction", TEVector2(0.0f, 1.0f))
    T_PROPERTY(float, Length, "Length", 100.0f)
    T_PROPERTY(bool, Enabled, "Enabled", true)
    T_PROPERTY(bool, DrawDebug, "Draw Debug", true)

    // Standard engine spatial query hit result
    TESpatialHitResult HitResult;

    SweepLine2DComponent() = default;
    virtual ~SweepLine2DComponent() override = default;

    virtual void Tick(float deltaTime) override;

    bool PerformSweep(Scene *scene);

    virtual void OnRender(class Renderer2D *renderer, const TEMatrix4 &worldModel,
                          const TERef<class Material> &material) const override;

    virtual TEString GetClassName() const override { return StaticClassName; }
};

#ifdef TE_EDITOR
T_REGISTER_COMPONENT(SweepLine2DComponent, "Sweep Line 2D Component")
T_REGISTER_PROPERTY(SweepLine2DComponent, TEVector2, Direction, "Direction")
T_REGISTER_PROPERTY(SweepLine2DComponent, float, Length, "Length")
T_REGISTER_PROPERTY(SweepLine2DComponent, bool, Enabled, "Enabled")
T_REGISTER_PROPERTY(SweepLine2DComponent, bool, DrawDebug, "Draw Debug")
T_REGISTER_PRESET(SweepLine2DComponent, "Sweep Line 2D", "Physics & Spatial Query",
                  [](EntityID id, EntityManager *em) { em->AddComponent<SweepLine2DComponent>(id); })
#endif
