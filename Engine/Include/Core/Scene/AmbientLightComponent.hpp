#pragma once
#include "Core/Scene/ComponentRegistry.hpp"
#include "GameFrameWork/TComponent.hpp"
#include "Renderer/TEColor.hpp"

namespace TE
{

class TE_API AmbientLightComponent : public TComponent
{
public:
    GENERATED_BODY(AmbientLightComponent)

    T_PROPERTY(TEColor, SkyColor, "Sky Color", TEColor(0.05f, 0.05f, 0.07f, 1.0f))
    T_PROPERTY(TEColor, HorizonColor, "Horizon Color", TEColor(0.12f, 0.12f, 0.15f, 1.0f))
    T_PROPERTY(TEColor, GroundColor, "Ground Color", TEColor(0.04f, 0.03f, 0.02f, 1.0f))
    T_PROPERTY(float, Intensity, "Intensity", 1.0f)
    T_PROPERTY(float, HorizonHeight, "Horizon Height", 0.5f)
    T_PROPERTY(float, HorizonSpread, "Horizon Spread", 0.2f)

    AmbientLightComponent() = default;
    virtual ~AmbientLightComponent() = default;

    const char *GetClassName() const override { return StaticClassName; }
};

#ifdef TE_EDITOR
T_REGISTER_COMPONENT(AmbientLightComponent, "Ambient Light Component")
T_REGISTER_PROPERTY(AmbientLightComponent, TEColor, SkyColor, "Sky Color")
T_REGISTER_PROPERTY(AmbientLightComponent, TEColor, HorizonColor, "Horizon Color")
T_REGISTER_PROPERTY(AmbientLightComponent, TEColor, GroundColor, "Ground Color")
T_REGISTER_PROPERTY(AmbientLightComponent, float, Intensity, "Intensity")
T_REGISTER_PROPERTY(AmbientLightComponent, float, HorizonHeight, "Horizon Height")
T_REGISTER_PROPERTY(AmbientLightComponent, float, HorizonSpread, "Horizon Spread")
#endif

} // namespace TE
