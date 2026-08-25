#pragma once

#include "CollisionComponent.hpp"
#include "Core/Scene/ComponentRegistry.hpp"

class TE_API CapsuleColliderComponent : public CollisionComponent
{
public:
    GENERATED_BODY(CapsuleColliderComponent)

    T_PROPERTY(TEVector2, Offset, "Offset", TEVector2(0.0f, 0.0f))
    T_PROPERTY(float, Radius, "Radius", 0.5f)
    T_PROPERTY(float, Height, "Height", 1.0f)

    CapsuleColliderComponent()
    {
        shape.type = CollisionType::Capsule;
    }

    virtual TEString GetClassName() const override { return StaticClassName; }

    virtual void OnUpdateShape(const TEMatrix4 &worldTransform) override
    {
        TEVector2 pos = {worldTransform[3].x + Offset.x, worldTransform[3].y + Offset.y};
        float scaleY = TEVector::Length(TEVector(worldTransform[1]));
        float scaleX = TEVector::Length(TEVector(worldTransform[0]));
        float worldRadius = Radius * scaleX;
        float halfSegment = std::max(0.0f, (Height * scaleY - worldRadius * 2.0f) * 0.5f);

        shape.capsule.point1 = {pos.x, pos.y - halfSegment};
        shape.capsule.point2 = {pos.x, pos.y + halfSegment};
        shape.capsule.radius = worldRadius;

        // Approximate AABB for broadphase
        shape.aabb.min = {pos.x - worldRadius, pos.y - halfSegment - worldRadius};
        shape.aabb.max = {pos.x + worldRadius, pos.y + halfSegment + worldRadius};
    }
};

#ifdef TE_EDITOR
T_REGISTER_COMPONENT(CapsuleColliderComponent, "Capsule Collider Component")
T_REGISTER_PROPERTY(CapsuleColliderComponent, TEVector2, Offset, "Offset")
T_REGISTER_PROPERTY(CapsuleColliderComponent, float, Radius, "Radius")
T_REGISTER_PROPERTY(CapsuleColliderComponent, float, Height, "Height")
T_REGISTER_PROPERTY(CapsuleColliderComponent, bool, isStatic, "Is Static")
T_REGISTER_PROPERTY(CapsuleColliderComponent, bool, isTrigger, "Is Trigger")
T_REGISTER_PRESET(CapsuleColliderComponent, "Capsule Collider 2D", "Physics & Collisions",
                  [](EntityID id, EntityManager *em) { em->AddComponent<CapsuleColliderComponent>(id); })
#endif
