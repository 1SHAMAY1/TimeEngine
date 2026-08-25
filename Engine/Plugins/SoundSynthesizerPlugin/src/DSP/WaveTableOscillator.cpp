#include "WaveTableOscillator.hpp"
#include "AudioDSPUtils.hpp"
#include <cmath>

namespace SoundStudio {

float WaveTableOscillator::ProcessSample(float sampleRate)
{
    float outSample = 0.0f;

    switch (m_Type)
    {
    case EWaveformType::Sine:
        outSample = std::sin(m_Phase * AudioDSPUtils::TWO_PI);
        break;

    case EWaveformType::Sawtooth:
        // Naive sawtooth: -1.0 to +1.0
        outSample = 2.0f * (m_Phase - std::floor(m_Phase + 0.5f));
        break;

    case EWaveformType::Square:
        outSample = (m_Phase < m_PulseWidth) ? 1.0f : -1.0f;
        break;

    case EWaveformType::Triangle:
        // Naive triangle
        outSample = 4.0f * std::fabs(m_Phase - std::floor(m_Phase + 0.75f) + 0.25f) - 1.0f;
        break;

    case EWaveformType::WhiteNoise:
        outSample = AudioDSPUtils::WhiteNoise();
        break;

    case EWaveformType::PinkNoise:
    {
        float white = AudioDSPUtils::WhiteNoise();
        m_B0 = 0.99886f * m_B0 + white * 0.0555179f;
        m_B1 = 0.99332f * m_B1 + white * 0.0750759f;
        m_B2 = 0.96900f * m_B2 + white * 0.1538520f;
        m_B3 = 0.86650f * m_B3 + white * 0.3104856f;
        m_B4 = 0.55000f * m_B4 + white * 0.5329522f;
        m_B5 = -0.7616f * m_B5 - white * 0.0168980f;
        outSample = (m_B0 + m_B1 + m_B2 + m_B3 + m_B4 + m_B5 + m_B6 + white * 0.5362f) * 0.11f;
        m_B6 = white * 0.115926f;
        break;
    }
    }

    // Increment phase
    m_Phase += m_Frequency / sampleRate;
    if (m_Phase >= 1.0f)
    {
        m_Phase -= std::floor(m_Phase);
    }

    return outSample;
}

void WaveTableOscillator::ProcessBlock(float sampleRate, AudioBufferBlock &outBlock)
{
    for (uint32_t i = 0; i < AUDIO_BLOCK_SIZE; ++i)
    {
        outBlock.Samples[i] = ProcessSample(sampleRate);
    }
}

} // namespace SoundStudio
