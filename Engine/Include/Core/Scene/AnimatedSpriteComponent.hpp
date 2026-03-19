#pragma once
#include "GameFrameWork/TComponent.hpp"
#include "Renderer/Texture.hpp"
#include <memory>
#include <vector>

namespace TE
{

struct AnimatedSpriteComponent : public TComponent
{
    std::vector<std::shared_ptr<TE::Texture>> Frames;
    float FrameTime = 0.1f;
    float TimeAccumulator = 0.0f;
    size_t CurrentFrame = 0;
    bool Loop = true;

    virtual const char *GetClassName() const override { return StaticClassName; }

    static constexpr const char *StaticClassName = "AnimatedSpriteComponent";
};

} // namespace TE