#pragma once
#include "CollisionTypes.hpp"
#include "ComponentRegistry.hpp"
#include "TComponent.hpp"

class CollisionComponent : public TComponent
{
public:
    GENERATED_BODY(CollisionComponent)
    T_EVENT_VISIBLE(TScriptEventType::CollisionEvent)

    CollisionShape shape;
    T_PROPERTY(bool, isStatic, "Is Static", false)
    T_PROPERTY(bool, isTrigger, "Is Trigger", false)
    bool collided = false;

    virtual TEString GetClassName() const override { return StaticClassName; }

    virtual void OnUpdateShape(const TEMatrix4 &worldTransform) {}
};
