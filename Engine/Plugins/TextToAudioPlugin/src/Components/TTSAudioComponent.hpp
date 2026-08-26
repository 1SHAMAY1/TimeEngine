#pragma once

#include "../TTSAudioEngine.hpp"
#include "Core/Scene/ComponentRegistry.hpp"
#include "GameFrameWork/TComponent.hpp"
#include "Utils/TEString.hpp"

class TTSAudioComponent : public TComponent
{
public:
    GENERATED_BODY(TTSAudioComponent)

    T_PROPERTY(TEString, DefaultText, "Default Text", "Hello from TimeEngine!")
    T_PROPERTY(float, Volume, "Volume", 1.0f)
    T_PROPERTY(float, Pitch, "Pitch", 1.0f)
    T_PROPERTY(float, Rate, "Speed / Rate", 1.0f)
    T_PROPERTY(bool, AutoSpeakOnStart, "Auto Speak On Start", false)
    T_PROPERTY(bool, EnableRichTextAcoustics, "Enable RichText Acoustics", true)
    T_PROPERTY(TEString, VoiceOverride, "Voice Override", "")

    TTSAudioComponent() = default;
    virtual ~TTSAudioComponent() = default;

    void OnInitialize() override { TComponent::OnInitialize(); }

    void OnAttach() override
    {
        TComponent::OnAttach();
        if (AutoSpeakOnStart && !DefaultText.empty())
        {
            Speak(DefaultText);
        }
    }

    void Speak(const TEString &text = "")
    {
        TTSSpeakRequest req;
        req.Text = text.empty() ? DefaultText : text;
        req.Volume = Volume;
        req.Pitch = Pitch;
        req.Rate = Rate;
        req.EnableRichTextAcoustics = EnableRichTextAcoustics;
        req.VoiceOverride = VoiceOverride;
        req.Mode = TTSSpeakMode::Async;

        TTSAudioEngine::Get().Speak(req);
    }

    void Stop() { TTSAudioEngine::Get().StopAll(); }

    const char *GetClassName() const override { return StaticClassName; }
};

#ifdef TE_EDITOR
T_REGISTER_COMPONENT(TTSAudioComponent, "TTS Audio Component")
T_REGISTER_PROPERTY(TTSAudioComponent, TEString, DefaultText, "Default Text")
T_REGISTER_PROPERTY(TTSAudioComponent, float, Volume, "Volume")
T_REGISTER_PROPERTY(TTSAudioComponent, float, Pitch, "Pitch")
T_REGISTER_PROPERTY(TTSAudioComponent, float, Rate, "Speed / Rate")
T_REGISTER_PROPERTY(TTSAudioComponent, bool, AutoSpeakOnStart, "Auto Speak On Start")
T_REGISTER_PROPERTY(TTSAudioComponent, bool, EnableRichTextAcoustics, "Enable RichText Acoustics")
T_REGISTER_PROPERTY(TTSAudioComponent, TEString, VoiceOverride, "Voice Override")
T_REGISTER_PRESET(TTSAudioComponent, "TTS Speaker", "Audio",
                  [](EntityID id, EntityManager *em) { em->AddComponent<TTSAudioComponent>(id); })
#endif
