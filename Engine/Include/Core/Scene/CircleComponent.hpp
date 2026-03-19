#pragma once
#include "ProceduralSpriteComponent.hpp"
#include "Utility/MathUtils.hpp"

namespace TE
{

class CircleComponent : public ProceduralSpriteComponent
{
public:
    float Radius = 0.5f;

    // Collision Data
    bool bHasCollision = true;
    bool bShowDebug = false;
    float Density = 1.0f;
    float Friction = 0.5f;

    virtual const char *GetClassName() const override { return StaticClassName; }

    static constexpr const char *StaticClassName = "CircleComponent";
};

} // namespace TE
