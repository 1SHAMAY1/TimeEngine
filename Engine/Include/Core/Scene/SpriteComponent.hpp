#pragma once
#include "Core/GameFrameWork/TComponent.hpp"
#include "Renderer/Texture.hpp"
#include "Renderer/TEColor.hpp"
#include <glm/glm.hpp>
#include <memory>

namespace TE {

class SpriteComponent : public TComponent {
public:
    SpriteComponent() = default;
    SpriteComponent(const std::shared_ptr<Texture>& texture,
                    const glm::vec2& position = {0,0},
                    const glm::vec2& size = {1,1},
                    float rotation = 0.0f,
                    const TEColor& color = TEColor::White())
        : Texture(texture), Position(position), Size(size), Rotation(rotation), Color(color) {}

    std::shared_ptr<Texture> Texture;
    glm::vec2 Position = {0, 0};
    glm::vec2 Size = {1, 1};
    float Rotation = 0.0f;
    TEColor Color = TEColor::White();
};

} // namespace TE 