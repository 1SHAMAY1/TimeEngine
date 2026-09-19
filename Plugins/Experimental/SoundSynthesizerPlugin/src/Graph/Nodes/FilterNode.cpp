#include "FilterNode.hpp"

namespace SoundStudio
{

FilterNode::FilterNode()
{
    AddInputPin("Audio In", EAudioPinType::AudioSignal, 0.0f);
    AddInputPin("Cutoff (Hz)", EAudioPinType::Float, 1200.0f);
    AddInputPin("Resonance (Q)", EAudioPinType::Float, 1.0f);
    AddInputPin("Cutoff Mod", EAudioPinType::AudioSignal, 0.0f);
    AddOutputPin("Audio Out", EAudioPinType::AudioSignal, 0.0f);
}

void FilterNode::ExecuteDSP(const AudioDSPContext &ctx)
{
    float cutoff = GetInputValueFloat("Cutoff (Hz)", ctx);
    float q = GetInputValueFloat("Resonance (Q)", ctx);

    const auto &inBlock = GetInputBuffer("Audio In");
    const auto &modBlock = GetInputBuffer("Cutoff Mod");
    auto &outBlock = GetOutputBuffer("Audio Out");

    for (uint32_t i = 0; i < AUDIO_BLOCK_SIZE; ++i)
    {
        float modCutoff = cutoff + modBlock.Samples[i] * 1000.0f;
        m_Filter.SetParameters(m_FilterType, modCutoff, q, (float)ctx.SampleRate);
        outBlock.Samples[i] = m_Filter.ProcessSample(inBlock.Samples[i]);
    }
}

} // namespace SoundStudio
