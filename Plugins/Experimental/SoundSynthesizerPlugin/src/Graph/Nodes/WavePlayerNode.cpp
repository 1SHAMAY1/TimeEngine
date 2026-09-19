#include "WavePlayerNode.hpp"
#include <cmath>

namespace SoundStudio
{

WavePlayerNode::WavePlayerNode()
{
    AddInputPin("Trigger In", EAudioPinType::Trigger, 0.0f);
    AddInputPin("Pitch", EAudioPinType::Float, 1.0f);
    AddInputPin("Loop", EAudioPinType::Float, 0.0f);
    AddOutputPin("Audio Out", EAudioPinType::AudioSignal, 0.0f);

    // Create 1 second default 440Hz wave for fallback
    m_SampleBuffer.resize(44100, 0.0f);
    for (size_t i = 0; i < m_SampleBuffer.size(); ++i)
    {
        m_SampleBuffer[i] = std::sin((float)i * 440.0f * 6.2831853f / 44100.0f);
    }
}

void WavePlayerNode::LoadWaveFile(const TEString &path)
{
    // Custom waveform buffer loader if requested
}

void WavePlayerNode::ExecuteDSP(const AudioDSPContext &ctx)
{
    if (IsInputTriggered("Trigger In"))
    {
        m_Playhead = 0.0f;
        m_IsPlaying = true;
    }

    float pitch = std::max(0.01f, GetInputValueFloat("Pitch", ctx));
    bool loop = GetInputValueFloat("Loop", ctx) > 0.5f;

    auto &outBlock = GetOutputBuffer("Audio Out");

    if (!m_IsPlaying || m_SampleBuffer.empty())
    {
        outBlock.Clear();
        return;
    }

    for (uint32_t i = 0; i < AUDIO_BLOCK_SIZE; ++i)
    {
        uint32_t idx = (uint32_t)m_Playhead;
        if (idx < m_SampleBuffer.size())
        {
            outBlock.Samples[i] = m_SampleBuffer[idx];
            m_Playhead += pitch;
        }
        else
        {
            if (loop)
            {
                m_Playhead = 0.0f;
                outBlock.Samples[i] = m_SampleBuffer[0];
            }
            else
            {
                outBlock.Samples[i] = 0.0f;
                m_IsPlaying = false;
            }
        }
    }
}

} // namespace SoundStudio
