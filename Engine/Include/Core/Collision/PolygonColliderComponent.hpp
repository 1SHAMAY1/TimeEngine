#pragma once
#include "CollisionComponent.hpp"
#include <vector>
#include "Core/Scene/ComponentRegistry.hpp"

namespace TE {

    class PolygonColliderComponent : public CollisionComponent {
    public:
        GENERATED_BODY(PolygonColliderComponent)

        T_PROPERTY(TEVector2, Offset, "Offset", TEVector2(0.0f, 0.0f))
        T_PROPERTY(bool, isStatic, "Is Static", false)
        T_PROPERTY(bool, isTrigger, "Is Trigger", false)

        std::vector<TEVector2> Vertices;

        PolygonColliderComponent() {
            shape.type = CollisionType::Polygon;
        }

        virtual const char* GetClassName() const override { return StaticClassName; }

        virtual void OnUpdateShape(const glm::mat4& worldTransform) override {
            shape.polygon.points.clear();
            for (const auto& v : Vertices) {
                glm::vec4 p = worldTransform * glm::vec4(v.x + Offset.x, v.y + Offset.y, 0.0f, 1.0f);
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

}
