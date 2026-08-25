#pragma once

#include "Core/Asset/Asset.hpp"
#include "Core/Scene/ComponentRegistry.hpp"
#include "GameFrameWork/TComponent.hpp"
#include "Renderer/Texture.hpp"
#include "Utils/MathUtils.hpp"

struct AnimationClip
{
    TEString Name = "Idle";
    AssetHandle AtlasHandle = 0;
    TEArray<int> FrameIndices;
    float FrameDuration = 0.1f;
    bool Loop = true;
    bool PingPong = false;
};

class TE_API AnimatedSpriteComponent : public TComponent
{
public:
    GENERATED_BODY(AnimatedSpriteComponent)

    T_PROPERTY(TEString, CurrentClipName, "Current Clip", "Idle")
    T_PROPERTY(float, PlaybackSpeed, "Playback Speed", 1.0f)
    T_PROPERTY(bool, AutoPlay, "Auto Play", true)
    T_PROPERTY(bool, FlipX, "Flip X", false)
    T_PROPERTY(bool, FlipY, "Flip Y", false)

    AnimatedSpriteComponent();
    virtual ~AnimatedSpriteComponent() override = default;

    virtual void OnAttach() override;
    virtual void Tick(float deltaTime) override;

    void Play(const TEString &clipName);
    void Pause();
    void Resume();
    void Stop();
    bool IsPlaying() const { return m_IsPlaying; }

    void AddClip(const AnimationClip &clip);
    const AnimationClip *GetClip(const TEString &name) const;

    int GetCurrentFrameIndex() const { return m_CurrentFrameIndex; }
    TEVector4 GetCurrentUVRect() const;

    virtual TEString GetClassName() const override { return StaticClassName; }

private:
    TEMap<TEString, AnimationClip> m_Clips;
    float m_FrameTimer = 0.0f;
    int m_CurrentFrameIndex = 0;
    bool m_IsPlaying = true;
    bool m_IsReversing = false;
};

#ifdef TE_EDITOR
T_REGISTER_COMPONENT(AnimatedSpriteComponent, "Animated Sprite Component")
T_REGISTER_PROPERTY(AnimatedSpriteComponent, TEString, CurrentClipName, "Current Clip")
T_REGISTER_PROPERTY(AnimatedSpriteComponent, float, PlaybackSpeed, "Playback Speed")
T_REGISTER_PROPERTY(AnimatedSpriteComponent, bool, AutoPlay, "Auto Play")
T_REGISTER_PROPERTY(AnimatedSpriteComponent, bool, FlipX, "Flip X")
T_REGISTER_PROPERTY(AnimatedSpriteComponent, bool, FlipY, "Flip Y")
T_REGISTER_PRESET(AnimatedSpriteComponent, "Animated Sprite", "2D Rendering",
                  [](EntityID id, EntityManager *em) { em->AddComponent<AnimatedSpriteComponent>(id); })
#endif
