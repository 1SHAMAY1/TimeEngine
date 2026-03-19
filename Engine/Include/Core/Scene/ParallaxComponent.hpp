#pragma once
#include "GameFrameWork/TComponent.hpp"
#include "Utility/MathUtils.hpp"

namespace TE
{

class ParallaxComponent : public TComponent
{
public:
    // Scroll speed relative to camera movement.
    // 1.0 = moves with camera (foreground), 0.0 = static (far background)
    TEVector2 ScrollFactor = {0.5f, 0.5f};

    ParallaxComponent() = default;
    ParallaxComponent(const TEVector2 &scrollFactor) : ScrollFactor(scrollFactor) {}

    virtual const char *GetClassName() const override { return StaticClassName; }

    static constexpr const char *StaticClassName = "ParallaxComponent";
};

} // namespace TE
