#pragma once
#include "CollisionComponent.hpp"
#include "Core/Scene/ComponentRegistry.hpp"

namespace TE
{

class BoxColliderComponent : public CollisionComponent
{
public:
    GENERATED_BODY(BoxColliderComponent)

    T_PROPERTY(TEVector2, Offset, "Offset", TEVector2(0.0f, 0.0f))
    T_PROPERTY(TEVector2, Size, "Size", TEVector2(1.0f, 1.0f))
    T_PROPERTY(bool, isStatic, "Is Static", false)
    T_PROPERTY(bool, isTrigger, "Is Trigger", false)

    BoxColliderComponent() { shape.type = CollisionType::AABB; }

    virtual const char *GetClassName() const override { return StaticClassName; }

    virtual void OnUpdateShape(const glm::mat4 &worldTransform) override
    {
        TEVector2 pos = {worldTransform[3].x + Offset.x, worldTransform[3].y + Offset.y};
        TEVector2 halfSize = {Size.x * 0.5f * glm::length(glm::vec3(worldTransform[0])),
                              Size.y * 0.5f * glm::length(glm::vec3(worldTransform[1]))};
        shape.aabb.min = {pos.x - halfSize.x, pos.y - halfSize.y};
        shape.aabb.max = {pos.x + halfSize.x, pos.y + halfSize.y};
    }
};

#ifdef TE_EDITOR
T_REGISTER_COMPONENT(BoxColliderComponent, "Box Collider Component")
T_REGISTER_PROPERTY(BoxColliderComponent, TEVector2, Offset, "Offset")
T_REGISTER_PROPERTY(BoxColliderComponent, TEVector2, Size, "Size")
T_REGISTER_PROPERTY(BoxColliderComponent, bool, isStatic, "Is Static")
T_REGISTER_PROPERTY(BoxColliderComponent, bool, isTrigger, "Is Trigger")
#endif

} // namespace TE
