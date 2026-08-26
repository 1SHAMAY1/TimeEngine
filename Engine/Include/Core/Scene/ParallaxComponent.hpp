#pragma once
#include "Core/Scene/ComponentRegistry.hpp"
#include "GameFrameWork/TComponent.hpp"
#include "Utils/MathUtils.hpp"

class ParallaxComponent : public TComponent
{
public:
    GENERATED_BODY(ParallaxComponent)

    T_PROPERTY(TEVector2, ScrollFactor, "Scroll Factor", TEVector2(0.5f, 0.5f))

    ParallaxComponent() = default;
    ParallaxComponent(const TEVector2 &scrollFactor) : ScrollFactor(scrollFactor) {}

    virtual TEString GetClassName() const override { return StaticClassName; }
};

#ifdef TE_EDITOR
T_REGISTER_COMPONENT(ParallaxComponent, "Parallax Component")
T_REGISTER_PROPERTY(ParallaxComponent, TEVector2, ScrollFactor, "Scroll Factor")
T_REGISTER_PRESET(ParallaxComponent, "Parallax Layer", "2D Rendering",
                  [](EntityID id, EntityManager *em) { em->AddComponent<ParallaxComponent>(id); })
#endif
