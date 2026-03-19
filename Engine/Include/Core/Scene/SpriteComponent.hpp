#pragma once
#include "GameFrameWork/TComponent.hpp"
#include "Renderer/Texture.hpp"
#include "Renderer/TEColor.hpp"
#include <glm/glm.hpp>
#include <memory>

namespace TE {

class SpriteComponent : public TComponent {
public:
    SpriteComponent() = default;
    SpriteComponent(const std::shared_ptr<Texture>& texture,
                    const TEColor& color = TEColor::White())
        : Texture(texture), Color(color) {}

    std::shared_ptr<Texture> Texture;
    TEColor Color = TEColor::White();

    virtual const char* GetClassName() const override { return StaticClassName; }

    static constexpr const char* StaticClassName = "SpriteComponent";
};

} // namespace TE 