#pragma once
#include "ComponentRegistry.hpp"
#include "TComponent.hpp"
#include "TEColor.hpp"
#include "Texture.hpp"

class SpriteComponent : public TComponent
{
public:
    GENERATED_BODY(SpriteComponent)

    T_PROPERTY(TEString, TexturePath, "Texture Path", "")
    T_PROPERTY(AssetHandle, TextureHandle, "Texture Asset", 0)
    T_PROPERTY(TEColor, Color, "Color", TEColor::White())

    SpriteComponent() = default;
    SpriteComponent(const TERef<class Texture> &texture, const TEColor &color = TEColor::White())
        : TextureRef(texture), Color(color)
    {
    }

    TERef<class Texture> TextureRef;

    virtual void OnRender(class Renderer2D *renderer, const TEMatrix4 &worldModel,
                          const TERef<class Material> &material) const override;

    virtual TEString GetClassName() const override { return StaticClassName; }
};

#ifdef TE_EDITOR
T_REGISTER_COMPONENT(SpriteComponent, "Sprite Component")
T_REGISTER_PROPERTY(SpriteComponent, TEString, TexturePath, "Texture Path")
T_REGISTER_PROPERTY(SpriteComponent, AssetHandle, TextureHandle, "Texture Asset")
T_REGISTER_PROPERTY(SpriteComponent, TEColor, Color, "Color")
T_REGISTER_PRESET(SpriteComponent, "Sprite", "2D Rendering",
                  [](EntityID id, EntityManager *em) { em->AddComponent<SpriteComponent>(id); })
#endif
