#include "OscillatorNode.hpp"

namespace SoundStudio
{

OscillatorNode::OscillatorNode()
{
    AddInputPin("Frequency", EAudioPinType::Float, 440.0f);
    AddInputPin("Pulse Width", EAudioPinType::Float, 0.5f);
    AddInputPin("FM In", EAudioPinType::AudioSignal, 0.0f);
    AddOutputPin("Audio Out", EAudioPinType::AudioSignal, 0.0f);
}

void OscillatorNode::ExecuteDSP(const AudioDSPContext &ctx)
{
    m_Osc.SetWaveform(m_Waveform);
    m_Osc.SetPulseWidth(GetInputValueFloat("Pulse Width", ctx));

    float baseFreq = GetInputValueFloat("Frequency", ctx);
    const auto &fmBuffer = GetInputBuffer("FM In");
    auto &outBlock = GetOutputBuffer("Audio Out");

    for (uint32_t i = 0; i < AUDIO_BLOCK_SIZE; ++i)
    {
        float freq = baseFreq + fmBuffer.Samples[i] * 100.0f;
        m_Osc.SetFrequency(freq);
        outBlock.Samples[i] = m_Osc.ProcessSample((float)ctx.SampleRate);
    }
}

} // namespace SoundStudio
