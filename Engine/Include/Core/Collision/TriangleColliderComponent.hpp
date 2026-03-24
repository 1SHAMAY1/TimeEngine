#pragma once
#include "CollisionComponent.hpp"
#include "Core/Scene/ComponentRegistry.hpp"

namespace TE
{

class TriangleColliderComponent : public CollisionComponent
{
public:
    GENERATED_BODY(TriangleColliderComponent)

    T_PROPERTY(TEVector2, Offset, "Offset", TEVector2(0.0f, 0.0f))
    T_PROPERTY(TEVector2, Vertices0, "V1", TEVector2(-0.5f, -0.5f))
    T_PROPERTY(TEVector2, Vertices1, "V2", TEVector2(0.5f, -0.5f))
    T_PROPERTY(TEVector2, Vertices2, "V3", TEVector2(0.0f, 0.5f))

    T_PROPERTY(bool, isStatic, "Is Static", false)
    T_PROPERTY(bool, isTrigger, "Is Trigger", false)

    TriangleColliderComponent() { shape.type = CollisionType::Triangle; }

    virtual const char *GetClassName() const override { return StaticClassName; }

    virtual void OnUpdateShape(const glm::mat4 &worldTransform) override
    {
        TEVector2 pts[3] = {Vertices0, Vertices1, Vertices2};
        for (int i = 0; i < 3; i++)
        {
            glm::vec4 p = worldTransform * glm::vec4(pts[i].x + Offset.x, pts[i].y + Offset.y, 0.0f, 1.0f);
            shape.triangle.points[i] = {p.x, p.y};
        }
    }
};

#ifdef TE_EDITOR
T_REGISTER_COMPONENT(TriangleColliderComponent, "Triangle Collider Component")
T_REGISTER_PROPERTY(TriangleColliderComponent, TEVector2, Offset, "Offset")
T_REGISTER_PROPERTY(TriangleColliderComponent, TEVector2, Vertices0, "V1")
T_REGISTER_PROPERTY(TriangleColliderComponent, TEVector2, Vertices1, "V2")
T_REGISTER_PROPERTY(TriangleColliderComponent, TEVector2, Vertices2, "V3")
T_REGISTER_PROPERTY(TriangleColliderComponent, bool, isStatic, "Is Static")
T_REGISTER_PROPERTY(TriangleColliderComponent, bool, isTrigger, "Is Trigger")
#endif

} // namespace TE
