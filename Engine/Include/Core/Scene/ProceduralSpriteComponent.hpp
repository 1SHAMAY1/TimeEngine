#pragma once
#include "Core/Scene/ComponentRegistry.hpp"
#include "GameFrameWork/TComponent.hpp"
#include "Renderer/TEColor.hpp"
#include "Utility/MathUtils.hpp"

namespace TE
{

class ProceduralSpriteComponent : public TComponent
{
public:
    GENERATED_BODY(ProceduralSpriteComponent)

    T_PROPERTY(TEColor, BaseColor, "Base Color", TEColor::White())
    T_PROPERTY(bool, bIsVisible, "Visible", true)

    virtual const char *GetClassName() const override { return StaticClassName; }
};

#ifdef TE_EDITOR
T_REGISTER_COMPONENT(ProceduralSpriteComponent, "Procedural Sprite Component")
T_REGISTER_PROPERTY(ProceduralSpriteComponent, TEColor, BaseColor, "Base Color")
T_REGISTER_PROPERTY(ProceduralSpriteComponent, bool, bIsVisible, "Visible")
#endif

} // namespace TE
