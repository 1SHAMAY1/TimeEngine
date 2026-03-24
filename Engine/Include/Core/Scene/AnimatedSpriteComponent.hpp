#pragma once
#include "Core/Scene/ComponentRegistry.hpp"
#include "GameFrameWork/TComponent.hpp"
#include "Renderer/Texture.hpp"
#include <memory>
#include <vector>

namespace TE
{

struct AnimatedSpriteComponent : public TComponent
{
    GENERATED_BODY(AnimatedSpriteComponent)

    T_PROPERTY(float, FrameTime, "Frame Time", 0.1f)
    T_PROPERTY(bool, Loop, "Loop", true)

    std::vector<std::shared_ptr<TE::Texture>> Frames;
    float TimeAccumulator = 0.0f;
    size_t CurrentFrame = 0;

    virtual const char *GetClassName() const override { return StaticClassName; }
};

#ifdef TE_EDITOR
T_REGISTER_COMPONENT(AnimatedSpriteComponent, "Animated Sprite Component")
T_REGISTER_PROPERTY(AnimatedSpriteComponent, float, FrameTime, "Frame Time")
T_REGISTER_PROPERTY(AnimatedSpriteComponent, bool, Loop, "Loop")
#endif

} // namespace TE