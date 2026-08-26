#pragma once

#include "../Graph/SoundGraphEvaluator.hpp"
#include "../Preprocessors/SoundGameplayTagFilter.hpp"
#include "Core/Audio/AudioEngine.hpp"
#include "Core/Scene/ComponentRegistry.hpp"
#include "GameFrameWork/TComponent.hpp"
#include "Utils/TEString.hpp"

class SoundGraphComponent : public TComponent
{
public:
    GENERATED_BODY(SoundGraphComponent)

    T_PROPERTY(TEString, GraphAssetPath, "Sound Graph Asset", "")
    T_PROPERTY(float, Volume, "Volume", 1.0f)
    T_PROPERTY(bool, AutoPlay, "Auto Play", true)
    T_PROPERTY(bool, Spatial, "2D Spatial Audio", true)

    SoundGraphComponent() = default;
    virtual ~SoundGraphComponent() = default;

    void OnInitialize() override
    {
        TComponent::OnInitialize();
        m_Evaluator = CreateRef<SoundStudio::SoundGraphEvaluator>();
    }

    void Tick(float deltaTime) override
    {
        if (AutoPlay && m_Evaluator && m_Evaluator->GetGraph())
        {
            if (!SoundStudio::SoundGameplayTagFilter::ShouldSynthesize())
                return;

            SoundStudio::StereoAudioBlock stereo;
            m_Evaluator->ProcessBlock(stereo);

            float tagVol = SoundStudio::SoundGameplayTagFilter::GetTagVolumeMultiplier();
            for (uint32_t i = 0; i < SoundStudio::AUDIO_BLOCK_SIZE; ++i)
            {
                stereo.Left.Samples[i] *= Volume * tagVol;
            }

            // Stream to audio engine
            AudioEngine::SubmitPCMFrames(stereo.Left.Samples, SoundStudio::AUDIO_BLOCK_SIZE,
                                         SoundStudio::DEFAULT_SAMPLE_RATE, 1);
        }
    }

    void SetParameter(const TEString &paramName, float value)
    {
        if (m_Evaluator && m_Evaluator->GetGraph())
        {
            m_Evaluator->GetGraph()->SetParameterFloat(paramName, value);
        }
    }

    void TriggerNote(int midiNote, float velocity = 1.0f)
    {
        if (m_Evaluator)
            m_Evaluator->TriggerNote(midiNote, velocity);
    }

    void TriggerNoteOff()
    {
        if (m_Evaluator)
            m_Evaluator->TriggerNoteOff();
    }

    virtual TEString GetClassName() const override { return StaticClassName; }

private:
    TERef<SoundStudio::SoundGraphEvaluator> m_Evaluator;
};

#ifdef TE_EDITOR
T_REGISTER_COMPONENT(SoundGraphComponent, "Sound Graph Component")
T_REGISTER_PROPERTY(SoundGraphComponent, TEString, GraphAssetPath, "Sound Graph Asset")
T_REGISTER_PROPERTY(SoundGraphComponent, float, Volume, "Volume")
T_REGISTER_PROPERTY(SoundGraphComponent, bool, AutoPlay, "Auto Play")
T_REGISTER_PROPERTY(SoundGraphComponent, bool, Spatial, "2D Spatial Audio")
T_REGISTER_PRESET(SoundGraphComponent, "Procedural Sound Graph", "Audio",
                  [](EntityID id, EntityManager *em) { em->AddComponent<SoundGraphComponent>(id); })
#endif
