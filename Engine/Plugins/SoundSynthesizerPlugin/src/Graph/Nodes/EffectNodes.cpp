#include "EffectNodes.hpp"

namespace SoundStudio {

// 1. Delay Node
DelayNode::DelayNode()
{
    AddInputPin("Audio In", EAudioPinType::AudioSignal, 0.0f);
    AddInputPin("Time (ms)", EAudioPinType::Float, 250.0f);
    AddInputPin("Feedback", EAudioPinType::Float, 0.4f);
    AddInputPin("Dry/Wet", EAudioPinType::Float, 0.3f);
    AddOutputPin("Audio Out", EAudioPinType::AudioSignal, 0.0f);
}

void DelayNode::ExecuteDSP(const AudioDSPContext &ctx)
{
    m_Delay.SetParameters(GetInputValueFloat("Time (ms)", ctx),
                          GetInputValueFloat("Feedback", ctx),
                          GetInputValueFloat("Dry/Wet", ctx),
                          (float)ctx.SampleRate);

    m_Delay.ProcessBlock(GetInputBuffer("Audio In"), GetOutputBuffer("Audio Out"));
}

// 2. Reverb Node
ReverbNode::ReverbNode()
{
    AddInputPin("Audio In", EAudioPinType::AudioSignal, 0.0f);
    AddInputPin("Room Size", EAudioPinType::Float, 0.5f);
    AddInputPin("Damping", EAudioPinType::Float, 0.5f);
    AddInputPin("Dry/Wet", EAudioPinType::Float, 0.33f);
    AddOutputPin("Audio Out", EAudioPinType::AudioSignal, 0.0f);
}

void ReverbNode::ExecuteDSP(const AudioDSPContext &ctx)
{
    float room = GetInputValueFloat("Room Size", ctx);
    float damp = GetInputValueFloat("Damping", ctx);
    float wet = GetInputValueFloat("Dry/Wet", ctx);

    m_Reverb.SetParameters(room, damp, wet, 1.0f - wet, 1.0f);

    StereoAudioBlock inStereo;
    const auto &inMono = GetInputBuffer("Audio In");
    inStereo.Left = inMono;
    inStereo.Right = inMono;

    StereoAudioBlock outStereo;
    m_Reverb.ProcessBlock(inStereo, outStereo);

    auto &outBlock = GetOutputBuffer("Audio Out");
    for (uint32_t i = 0; i < AUDIO_BLOCK_SIZE; ++i)
    {
        outBlock.Samples[i] = (outStereo.Left.Samples[i] + outStereo.Right.Samples[i]) * 0.5f;
    }
}

} // namespace SoundStudio
