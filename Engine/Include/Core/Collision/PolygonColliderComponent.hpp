#pragma once
#include "CollisionComponent.hpp"
#include "Core/Scene/ComponentRegistry.hpp"
#include <vector>

namespace TE
{

class PolygonColliderComponent : public CollisionComponent
{
public:
    GENERATED_BODY(PolygonColliderComponent)

    T_PROPERTY(TEVector2, Offset, "Offset", TEVector2(0.0f, 0.0f))

    std::vector<TEVector2> Vertices;

    PolygonColliderComponent() { shape.type = CollisionType::Polygon; }

    virtual const char *GetClassName() const override { return StaticClassName; }

    virtual void OnUpdateShape(const TEMatrix4 &worldTransform) override
    {
        shape.polygon.points.clear();
        for (const auto &v : Vertices)
        {
            TEVector4 p = worldTransform * TEVector4(v.x + Offset.x, v.y + Offset.y, 0.0f, 1.0f);
            shape.polygon.points.push_back({p.x, p.y});
        }
    }
};

#ifdef TE_EDITOR
T_REGISTER_COMPONENT(PolygonColliderComponent, "Polygon Collider Component")
T_REGISTER_PROPERTY(PolygonColliderComponent, TEVector2, Offset, "Offset")
T_REGISTER_PROPERTY(PolygonColliderComponent, bool, isStatic, "Is Static")
T_REGISTER_PROPERTY(PolygonColliderComponent, bool, isTrigger, "Is Trigger")
#endif

} // namespace TE
