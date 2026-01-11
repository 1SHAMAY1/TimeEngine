#pragma once
#include "Core/GameFrameWork/TComponent.hpp"
#include "Renderer/Texture.hpp"
#include <vector>
#include <memory>

struct AnimatedSpriteComponent : public TComponent {
    std::vector<std::shared_ptr<TE::Texture>> Frames;
    float FrameTime = 0.1f;
    float TimeAccumulator = 0.0f;
    size_t CurrentFrame = 0;
    bool Loop = true;
}; 