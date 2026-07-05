#pragma once
#include "CollisionTypes.hpp"
#include "GameFrameWork/TComponent.hpp"
#include "Core/Scene/ComponentRegistry.hpp"

namespace TE
{

class CollisionComponent : public TComponent
{
public:
    GENERATED_BODY(CollisionComponent)

    CollisionShape shape;
    T_PROPERTY(bool, isStatic, "Is Static", false)
    T_PROPERTY(bool, isTrigger, "Is Trigger", false)
    bool collided = false;

    virtual const char *GetClassName() const override { return StaticClassName; }

    virtual void OnUpdateShape(const TEMatrix4 &worldTransform) {}
};

} // namespace TE
