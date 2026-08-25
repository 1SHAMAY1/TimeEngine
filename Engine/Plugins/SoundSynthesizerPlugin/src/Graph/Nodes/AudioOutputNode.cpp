#include "AudioOutputNode.hpp"

namespace SoundStudio {

AudioOutputNode::AudioOutputNode()
{
    AddInputPin("Left In", EAudioPinType::AudioSignal, 0.0f);
    AddInputPin("Right In", EAudioPinType::AudioSignal, 0.0f);
    AddInputPin("Master Volume", EAudioPinType::Float, 1.0f);
}

void AudioOutputNode::ExecuteDSP(const AudioDSPContext &ctx)
{
    float vol = GetInputValueFloat("Master Volume", ctx);
    const auto &inL = GetInputBuffer("Left In");
    const auto &inR = GetInputBuffer("Right In");

    for (uint32_t i = 0; i < AUDIO_BLOCK_SIZE; ++i)
    {
        m_LeftBlock.Samples[i] = inL.Samples[i] * vol;
        m_RightBlock.Samples[i] = inR.Samples[i] * vol;
    }
}

} // namespace SoundStudio
