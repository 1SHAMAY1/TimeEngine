#pragma once
#include "GameFrameWork/TComponent.hpp"
#include "Renderer/TEColor.hpp"
#include "Renderer/Texture.hpp"
#include <glm/glm.hpp>
#include <memory>
#include "Core/Scene/ComponentRegistry.hpp"

namespace TE
{

class SpriteComponent : public TComponent
{
public:
    GENERATED_BODY(SpriteComponent)

    T_PROPERTY(TEColor, Color, "Color", TEColor::White())

    SpriteComponent() = default;
    SpriteComponent(const std::shared_ptr<Texture> &texture, const TEColor &color = TEColor::White())
        : Texture(texture), Color(color)
    {
    }

    std::shared_ptr<Texture> Texture;

    virtual const char *GetClassName() const override { return StaticClassName; }
};

#ifdef TE_EDITOR
T_REGISTER_COMPONENT(SpriteComponent, "Sprite Component")
T_REGISTER_PROPERTY(SpriteComponent, TEColor, Color, "Color")
#endif

} // namespace TE