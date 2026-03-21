#pragma once
#include "GameFrameWork/TComponent.hpp"
#include "Renderer/TEColor.hpp"

namespace TE
{

class TE_API AmbientLightComponent : public TComponent
{
public:
    TEColor SkyColor = TEColor(0.05f, 0.05f, 0.07f, 1.0f);     // Top
    TEColor HorizonColor = TEColor(0.12f, 0.12f, 0.15f, 1.0f); // Middle
    TEColor GroundColor = TEColor(0.04f, 0.03f, 0.02f, 1.0f);  // Bottom

    float Intensity = 1.0f;
    float HorizonHeight = 0.5f; // 0.0 to 1.0
    float HorizonSpread = 0.2f; // Thickness of horizon band

    AmbientLightComponent() = default;
    virtual ~AmbientLightComponent() = default;

    const char *GetClassName() const override { return "AmbientLightComponent"; }
};

} // namespace TE
