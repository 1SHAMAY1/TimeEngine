#pragma once
#include "Core/Scene/ComponentRegistry.hpp"
#include "GameFrameWork/TComponent.hpp"
#include "Renderer/TEColor.hpp"
#include "Renderer/Texture.hpp"
#include <glm/glm.hpp>
#include <memory>


class SpriteComponent : public TComponent
{
public:
    GENERATED_BODY(SpriteComponent)

    T_PROPERTY(TEColor, Color, "Color", TEColor::White())

    SpriteComponent() = default;
    SpriteComponent(const TERef<Texture> &texture, const TEColor &color = TEColor::White())
        : Texture(texture), Color(color)
    {
    }

    TERef<Texture> Texture;

    virtual TEString GetClassName() const override { return StaticClassName; }
};

#ifdef TE_EDITOR
T_REGISTER_COMPONENT(SpriteComponent, "Sprite Component")
T_REGISTER_PROPERTY(SpriteComponent, TEColor, Color, "Color")
T_REGISTER_PRESET(SpriteComponent, "Sprite", "2D Rendering",
                  [](EntityID id, EntityManager *em) { em->AddComponent<SpriteComponent>(id); })
#endif

