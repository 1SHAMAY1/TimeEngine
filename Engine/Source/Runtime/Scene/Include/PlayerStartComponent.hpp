#pragma once
#include "ComponentRegistry.hpp"
#include "TComponent.hpp"
#include "Renderer2D.hpp"
#include "MathUtils.hpp"

TE_CLASS()
class TE_API PlayerStartComponent : public TComponent
{
public:
    GENERATED_BODY(PlayerStartComponent)

    T_PROPERTY(int, PlayerIndex, "Player Index", 0)
    T_PROPERTY(bool, bIsEnabled, "Enabled", true)

    PlayerStartComponent() = default;
    virtual ~PlayerStartComponent() override = default;

    virtual void OnRender(class Renderer2D *renderer, const TEMatrix4 &worldModel,
                          const TERef<class Material> &material) const override
    {
        if (!renderer)
            return;

        // Render editor spawn point indicator circle and arrow
        TEVector2 worldPos = {worldModel.m[3][0], worldModel.m[3][1]};
        renderer->SubmitCircle(worldPos, 0.4f, TEColor(0.2f, 0.8f, 1.0f, 0.8f), material);
        renderer->SubmitQuad(TEMatrix4::Scale(worldModel, TEVector(0.5f, 0.5f, 1.0f)), TEColor(0.2f, 0.8f, 1.0f, 0.5f), material);
    }

    virtual TEString GetClassName() const override { return StaticClassName; }
};

#ifdef TE_EDITOR
T_REGISTER_COMPONENT(PlayerStartComponent, "Player Start Component")
T_REGISTER_PROPERTY(PlayerStartComponent, int, PlayerIndex, "Player Index")
T_REGISTER_PROPERTY(PlayerStartComponent, bool, bIsEnabled, "Enabled")
T_REGISTER_PRESET(PlayerStart, "Player Start", "Gameplay",
                  [](EntityID id, EntityManager *em) { em->AddComponent<PlayerStartComponent>(id); })
#endif
