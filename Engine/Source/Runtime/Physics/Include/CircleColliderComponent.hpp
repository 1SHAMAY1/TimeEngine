#pragma once
#include "CollisionComponent.hpp"
#include "ComponentRegistry.hpp"
#include "Renderer2D.hpp"

class CircleColliderComponent : public CollisionComponent
{
public:
    GENERATED_BODY(CircleColliderComponent)

    T_PROPERTY(TEVector2, Offset, "Offset", TEVector2(0.0f, 0.0f))
    T_PROPERTY(float, Radius, "Radius", 0.5f)

    CircleColliderComponent() { shape.type = CollisionType::Circle; }

    virtual TEString GetClassName() const override { return StaticClassName; }

    virtual void OnUpdateShape(const TEMatrix4 &worldTransform) override
    {
        shape.circle.center = {worldTransform[3].x + Offset.x, worldTransform[3].y + Offset.y};
        shape.circle.radius = Radius * TEVector::Length(TEVector(worldTransform[0]));
    }

    virtual void OnRender(class Renderer2D *renderer, const TEMatrix4 &worldTransform,
                          const TERef<class Material> &material) const override
    {
        if (!renderer)
            return;
        TEVector4 pos4 = worldTransform * TEVector4(Offset.x, Offset.y, 0.0f, 1.0f);
        renderer->SubmitCircleOutline(TEVector2(pos4.x, pos4.y), Radius, 1.5f, TEColor(0.0f, 1.0f, 0.0f, 0.75f));
    }
};

#ifdef TE_EDITOR
T_REGISTER_COMPONENT(CircleColliderComponent, "Circle Collider Component")
T_REGISTER_PROPERTY(CircleColliderComponent, TEVector2, Offset, "Offset")
T_REGISTER_PROPERTY(CircleColliderComponent, float, Radius, "Radius")
T_REGISTER_PROPERTY(CircleColliderComponent, bool, isStatic, "Is Static")
T_REGISTER_PROPERTY(CircleColliderComponent, bool, isTrigger, "Is Trigger")
T_REGISTER_PRESET(CircleColliderComponent, "Circle Collider 2D", "Physics & Collisions",
                  [](EntityID id, EntityManager *em) { em->AddComponent<CircleColliderComponent>(id); })
#endif
