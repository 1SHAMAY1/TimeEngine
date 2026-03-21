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

    // Point & Spot
    float Radius = 5.0f;

    // Spot
    float InnerAngle = 15.0f;
    float OuterAngle = 30.0f;
    TEVector2 Direction = {0.0f, -1.0f};

    // Line
    TEVector2 LineOffset = {0.0f, 2.0f}; // Relative to position
    float Width = 0.5f;
    float FalloffExponent = 2.0f; // 1.0 = linear, 2.0 = quadratic

    LightComponent() = default;
    virtual ~LightComponent() = default;

    const char *GetClassName() const override { return "LightComponent"; }

    bool ContainsPoint(const glm::mat4 &worldModel, const TEVector2 &point) const override
    {
        // Lights are clickable within a small radius around their position
        glm::vec2 pos = {worldModel[3].x, worldModel[3].y};
        float clickRadius = 0.5f; // World-space click radius
        float dx = point.x - pos.x, dy = point.y - pos.y;
        return (dx * dx + dy * dy) <= clickRadius * clickRadius;
    }
};
} // namespace TE
