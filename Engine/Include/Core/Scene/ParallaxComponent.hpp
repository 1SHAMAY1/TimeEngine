#pragma once
#include "GameFrameWork/TComponent.hpp"
#include "Utility/MathUtils.hpp"
#include "Core/Scene/ComponentRegistry.hpp"

namespace TE
{

class ParallaxComponent : public TComponent
{
public:
    GENERATED_BODY(ParallaxComponent)

    T_PROPERTY(TEVector2, ScrollFactor, "Scroll Factor", TEVector2(0.5f, 0.5f))

    ParallaxComponent() = default;
    ParallaxComponent(const TEVector2 &scrollFactor) : ScrollFactor(scrollFactor) {}

    virtual const char *GetClassName() const override { return StaticClassName; }
};

#ifdef TE_EDITOR
T_REGISTER_COMPONENT(ParallaxComponent, "Parallax Component")
T_REGISTER_PROPERTY(ParallaxComponent, TEVector2, ScrollFactor, "Scroll Factor")
#endif

} // namespace TE
