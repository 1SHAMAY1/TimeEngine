#pragma once

#include "Core/Audio/AudioEngine.hpp"
#include "Core/Scene/ComponentRegistry.hpp"
#include "GameFrameWork/TComponent.hpp"
#include "Utils/TEString.hpp"

class TE_API AudioSource2DComponent : public TComponent
{
public:
    GENERATED_BODY(AudioSource2DComponent)

    T_PROPERTY(TEString, ClipPath, "Audio Clip Path", "")
    T_PROPERTY(float, Volume, "Volume", 1.0f)
    T_PROPERTY(float, Pitch, "Pitch", 1.0f)
    T_PROPERTY(float, Pan, "Pan", 0.0f)
    T_PROPERTY(bool, Loop, "Loop", false)
    T_PROPERTY(bool, PlayOnStart, "Play On Start", false)
    T_PROPERTY(bool, Spatial, "2D Spatial Audio", true)
    T_PROPERTY(float, MinDistance, "Min Distance", 1.0f)
    T_PROPERTY(float, MaxDistance, "Max Distance", 50.0f)
    T_PROPERTY(float, RollOff, "Roll-Off Factor", 1.0f)

    AudioSource2DComponent() = default;
    virtual ~AudioSource2DComponent();

    void OnInitialize() override;
    void OnAttach() override;
    void OnDetach() override;
    void Tick(float deltaTime) override;

    // Runtime playback controls
    void Play(AudioPlayMode mode = AudioPlayMode::Async);
    void Stop();
    void Pause();
    void Resume();
    bool IsPlaying() const;

    virtual TEString GetClassName() const override { return StaticClassName; }

private:
    AudioClipHandle m_ClipHandle = InvalidAudioClip;
    bool m_WasPlaying = false;
};

#ifdef TE_EDITOR
T_REGISTER_COMPONENT(AudioSource2DComponent, "Audio Source 2D Component")
T_REGISTER_PROPERTY(AudioSource2DComponent, TEString, ClipPath, "Audio Clip Path")
T_REGISTER_PROPERTY(AudioSource2DComponent, float, Volume, "Volume")
T_REGISTER_PROPERTY(AudioSource2DComponent, float, Pitch, "Pitch")
T_REGISTER_PROPERTY(AudioSource2DComponent, float, Pan, "Pan")
T_REGISTER_PROPERTY(AudioSource2DComponent, bool, Loop, "Loop")
T_REGISTER_PROPERTY(AudioSource2DComponent, bool, PlayOnStart, "Play On Start")
T_REGISTER_PROPERTY(AudioSource2DComponent, bool, Spatial, "2D Spatial Audio")
T_REGISTER_PROPERTY(AudioSource2DComponent, float, MinDistance, "Min Distance")
T_REGISTER_PROPERTY(AudioSource2DComponent, float, MaxDistance, "Max Distance")
T_REGISTER_PROPERTY(AudioSource2DComponent, float, RollOff, "Roll-Off Factor")
T_REGISTER_PRESET(AudioSource2DComponent, "Audio Source 2D", "Audio",
                  [](EntityID id, EntityManager *em) { em->AddComponent<AudioSource2DComponent>(id); })
#endif
