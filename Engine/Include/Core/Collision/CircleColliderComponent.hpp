#pragma once
#include "CollisionComponent.hpp"
#include "Core/Scene/ComponentRegistry.hpp"

namespace TE {

    class CircleColliderComponent : public CollisionComponent {
    public:
        GENERATED_BODY(CircleColliderComponent)

        T_PROPERTY(TEVector2, Offset, "Offset", TEVector2(0.0f, 0.0f))
        T_PROPERTY(float, Radius, "Radius", 0.5f)
        T_PROPERTY(bool, isStatic, "Is Static", false)
        T_PROPERTY(bool, isTrigger, "Is Trigger", false)

        CircleColliderComponent() {
            shape.type = CollisionType::Circle;
        }

        virtual const char* GetClassName() const override { return StaticClassName; }

        virtual void OnUpdateShape(const glm::mat4& worldTransform) override {
            shape.circle.center = {worldTransform[3].x + Offset.x, worldTransform[3].y + Offset.y};
            shape.circle.radius = Radius * glm::length(glm::vec3(worldTransform[0]));
        }
    };

#ifdef TE_EDITOR
T_REGISTER_COMPONENT(CircleColliderComponent, "Circle Collider Component")
T_REGISTER_PROPERTY(CircleColliderComponent, TEVector2, Offset, "Offset")
T_REGISTER_PROPERTY(CircleColliderComponent, float, Radius, "Radius")
T_REGISTER_PROPERTY(CircleColliderComponent, bool, isStatic, "Is Static")
T_REGISTER_PROPERTY(CircleColliderComponent, bool, isTrigger, "Is Trigger")
#endif

}
