#pragma once
#include "ProceduralSpriteComponent.hpp"
#include "Utility/MathUtils.hpp"

namespace TE
{

class TriangleComponent : public ProceduralSpriteComponent
{
public:
    TEVector2 Point1 = {-0.5f, -0.5f};
    TEVector2 Point2 = {0.5f, -0.5f};
    TEVector2 Point3 = {0.0f, 0.5f};

    // Collision Data
    bool bHasCollision = true;
    bool bShowDebug = false;
    float Density = 1.0f;
    float Friction = 0.5f;

    virtual const char *GetClassName() const override { return StaticClassName; }

    static constexpr const char *StaticClassName = "TriangleComponent";
};

} // namespace TE
