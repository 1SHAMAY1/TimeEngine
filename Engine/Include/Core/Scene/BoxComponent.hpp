#pragma once
#include "ProceduralSpriteComponent.hpp"
#include "Utility/MathUtils.hpp"

namespace TE
{

class BoxComponent : public ProceduralSpriteComponent
{
public:
    TEVector2 Size = {1.0f, 1.0f};

    // Collision Data
    bool bHasCollision = true;
    bool bShowDebug = false;
    float Density = 1.0f;
    float Friction = 0.5f;

    virtual const char *GetClassName() const override { return StaticClassName; }

    static constexpr const char *StaticClassName = "BoxComponent";
};

} // namespace TE
