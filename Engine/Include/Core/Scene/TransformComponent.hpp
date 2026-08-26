#pragma once
#include "GameFrameWork/TComponent.hpp"
#include "Utils/MathUtils.hpp"

class TransformComponent : public TComponent
{
public:
    EntityID Parent = 0;
    TEArray<EntityID> Children;

    TransformComponent() = default;
    TransformComponent(const TEVector &position) { Transform.Position = position; }

    virtual TEString GetClassName() const override { return StaticClassName; }

    inline static const TEString StaticClassName = "TransformComponent";
};

#ifdef TE_EDITOR
#include "Core/Scene/ComponentRegistry.hpp"
T_REGISTER_COMPONENT(TransformComponent, "Transform Component")
T_REGISTER_PROPERTY(TransformComponent, EntityID, Parent, "Parent")
T_COMPONENT_INTERNAL(TransformComponent)

inline static bool s_TransformComp_Props_Reg = []()
{
    auto &reg = ComponentRegistry::Get();
    reg.RegisterProperty<TransformComponent, TEVector>(
        "TransformComponent", "Position", "Position",
        [](void *inst) { return &static_cast<TransformComponent *>(inst)->Transform.Position; });
    reg.RegisterProperty<TransformComponent, TERotator>(
        "TransformComponent", "Rotation", "Rotation",
        [](void *inst) { return &static_cast<TransformComponent *>(inst)->Transform.Rotation; });
    reg.RegisterProperty<TransformComponent, TEScale>(
        "TransformComponent", "Scale", "Scale",
        [](void *inst) { return &static_cast<TransformComponent *>(inst)->Transform.Scale; });
    return true;
}();
#endif
