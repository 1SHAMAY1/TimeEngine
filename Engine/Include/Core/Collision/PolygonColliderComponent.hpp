#pragma once
#include "CollisionComponent.hpp"
#include "Core/Scene/ComponentRegistry.hpp"
#include <vector>

class PolygonColliderComponent : public CollisionComponent
{
public:
    GENERATED_BODY(PolygonColliderComponent)

    T_PROPERTY(TEVector2, Offset, "Offset", TEVector2(0.0f, 0.0f))

    TEArray<TEVector2> Vertices;

    PolygonColliderComponent() { shape.type = CollisionType::Polygon; }

    virtual TEString GetClassName() const override { return StaticClassName; }

    virtual void OnUpdateShape(const TEMatrix4 &worldTransform) override
    {
        shape.polygon.points.Clear();
        for (const auto &v : Vertices)
        {
            TEVector4 p = worldTransform * TEVector4(v.x + Offset.x, v.y + Offset.y, 0.0f, 1.0f);
            shape.polygon.points.Add({p.x, p.y});
        }
    }
};

#ifdef TE_EDITOR
T_REGISTER_COMPONENT(PolygonColliderComponent, "Polygon Collider Component")
T_REGISTER_PROPERTY(PolygonColliderComponent, TEVector2, Offset, "Offset")
T_REGISTER_PROPERTY(PolygonColliderComponent, bool, isStatic, "Is Static")
T_REGISTER_PROPERTY(PolygonColliderComponent, bool, isTrigger, "Is Trigger")
T_REGISTER_PRESET(PolygonColliderComponent, "Polygon Collider 2D", "Physics & Collisions",
                  [](EntityID id, EntityManager *em) { em->AddComponent<PolygonColliderComponent>(id); })
#endif
