#pragma once
#include "GameFrameWork/TComponent.hpp"
#include "Renderer/TEColor.hpp"
#include "Utility/MathUtils.hpp"
#include "Core/Scene/ComponentRegistry.hpp"

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
    GENERATED_BODY(LightComponent)

    T_PROPERTY(float, Intensity, "Intensity", 1.0f, Intensity, Intensity)
    T_PROPERTY(TEColor, Color, "Color", TEColor::White())
    T_PROPERTY(float, Radius, "Radius", 5.0f)
    T_PROPERTY(float, InnerAngle, "Inner Angle", 15.0f)
    T_PROPERTY(float, OuterAngle, "Outer Angle", 30.0f)
    T_PROPERTY(TEVector2, Direction, "Direction", TEVector2(0.0f, -1.0f))
    T_PROPERTY(TEVector2, LineOffset, "Line Offset", TEVector2(0.0f, 2.0f))
    T_PROPERTY(float, Width, "Width", 0.5f)
    T_PROPERTY(float, FalloffExponent, "Falloff Exponent", 2.0f)

    TELightType Type = TELightType::Point;

    LightComponent() = default;
    virtual ~LightComponent() = default;

    const char *GetClassName() const override { return StaticClassName; }

    bool ContainsPoint(const glm::mat4 &worldModel, const TEVector2 &point) const override
    {
        glm::vec2 pos = {worldModel[3].x, worldModel[3].y};
        float clickRadius = 0.5f;
        float dx = point.x - pos.x, dy = point.y - pos.y;
        return (dx * dx + dy * dy) <= clickRadius * clickRadius;
    }
};

#ifdef TE_EDITOR
T_ENUM(TELightType, {"Point", TELightType::Point}, {"Spot", TELightType::Spot}, {"Line", TELightType::Line})

T_REGISTER_COMPONENT(LightComponent, "Light Component")
T_REGISTER_ENUM_PROPERTY(LightComponent, TELightType, Type, "Light Type")
T_REGISTER_PROPERTY(LightComponent, float, Intensity, "Intensity")
T_REGISTER_PROPERTY(LightComponent, TEColor, Color, "Color")

T_REGISTER_PROPERTY_COND(LightComponent, float, Radius, "Radius", [](void* inst) { 
    return ((LightComponent*)inst)->Type == TELightType::Point || ((LightComponent*)inst)->Type == TELightType::Spot; 
})
T_REGISTER_PROPERTY_COND(LightComponent, float, FalloffExponent, "Falloff Exponent", [](void* inst) { 
    return ((LightComponent*)inst)->Type != TELightType::Line; 
})

T_REGISTER_PROPERTY_COND(LightComponent, float, InnerAngle, "Inner Angle", [](void* inst) { 
    return ((LightComponent*)inst)->Type == TELightType::Spot; 
})
T_REGISTER_PROPERTY_COND(LightComponent, float, OuterAngle, "Outer Angle", [](void* inst) { 
    return ((LightComponent*)inst)->Type == TELightType::Spot; 
})
T_REGISTER_PROPERTY_COND(LightComponent, TEVector2, Direction, "Direction", [](void* inst) { 
    return ((LightComponent*)inst)->Type == TELightType::Spot; 
})

T_REGISTER_PROPERTY_COND(LightComponent, TEVector2, LineOffset, "Line Offset", [](void* inst) { 
    return ((LightComponent*)inst)->Type == TELightType::Line; 
})
T_REGISTER_PROPERTY_COND(LightComponent, float, Width, "Width", [](void* inst) { 
    return ((LightComponent*)inst)->Type == TELightType::Line; 
})

// Entity presets registered here - no need to touch EditorLayer.cpp for new light types
T_REGISTER_PRESET(PointLight, "Point Light", "Lights", ([](::TE::EntityID id, ::TE::EntityManager* em) {
    em->AddComponent<LightComponent>(id);
}))
T_REGISTER_PRESET(SpotLight, "Spot Light", "Lights", ([](::TE::EntityID id, ::TE::EntityManager* em) {
    auto* l = em->AddComponent<LightComponent>(id);
    if (l) l->Type = TELightType::Spot;
}))
T_REGISTER_PRESET(LineLight, "Line Light", "Lights", ([](::TE::EntityID id, ::TE::EntityManager* em) {
    auto* l = em->AddComponent<LightComponent>(id);
    if (l) l->Type = TELightType::Line;
}))
#endif

} // namespace TE
