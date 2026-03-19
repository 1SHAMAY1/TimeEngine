#pragma once
#include "GameFrameWork/TComponent.hpp"
#include "Renderer/TEColor.hpp"
#include "Utility/MathUtils.hpp"

namespace TE
{

enum class TELightType
{
    Point = 0,
    Spot = 1,
    Line = 2
};

class TE_API LightComponent : public TComponent
{
public:
    TELightType Type = TELightType::Point;
    TEColor Color = TEColor::White();
    float Intensity = 1.0f;
    bool bIsVisible = true;

    // Point & Spot
    float Radius = 5.0f;

    // Spot
    float InnerAngle = 15.0f;
    float OuterAngle = 30.0f;
    TEVector2 Direction = {0.0f, -1.0f};

    // Line
    TEVector2 LineOffset = {0.0f, 2.0f}; // Relative to position
    float Width = 0.5f;

    LightComponent() = default;
    virtual ~LightComponent() = default;

    const char *GetClassName() const override { return "LightComponent"; }
};
} // namespace TE
