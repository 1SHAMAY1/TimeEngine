#include "BiquadFilter.hpp"
#include "AudioDSPUtils.hpp"
#include <cmath>

namespace SoundStudio
{

void BiquadFilter::SetParameters(EFilterType type, float cutoffHz, float q, float sampleRate)
{
    cutoffHz = AudioDSPUtils::Clamp(cutoffHz, 10.0f, sampleRate * 0.49f);
    q = AudioDSPUtils::Clamp(q, 0.1f, 30.0f);

    float omega = AudioDSPUtils::TWO_PI * cutoffHz / sampleRate;
    float sinOmega = std::sin(omega);
    float cosOmega = std::cos(omega);
    float alpha = sinOmega / (2.0f * q);

    float a0 = 1.0f;

    switch (type)
    {
    case EFilterType::LowPass12:
    case EFilterType::LowPass24:
        m_B0 = (1.0f - cosOmega) * 0.5f;
        m_B1 = 1.0f - cosOmega;
        m_B2 = (1.0f - cosOmega) * 0.5f;
        a0 = 1.0f + alpha;
        m_A1 = -2.0f * cosOmega;
        m_A2 = 1.0f - alpha;
        break;

    case EFilterType::HighPass:
        m_B0 = (1.0f + cosOmega) * 0.5f;
        m_B1 = -(1.0f + cosOmega);
        m_B2 = (1.0f + cosOmega) * 0.5f;
        a0 = 1.0f + alpha;
        m_A1 = -2.0f * cosOmega;
        m_A2 = 1.0f - alpha;
        break;

    case EFilterType::BandPass:
        m_B0 = alpha;
        m_B1 = 0.0f;
        m_B2 = -alpha;
        a0 = 1.0f + alpha;
        m_A1 = -2.0f * cosOmega;
        m_A2 = 1.0f - alpha;
        break;

    case EFilterType::Notch:
        m_B0 = 1.0f;
        m_B1 = -2.0f * cosOmega;
        m_B2 = 1.0f;
        a0 = 1.0f + alpha;
        m_A1 = -2.0f * cosOmega;
        m_A2 = 1.0f - alpha;
        break;

    case EFilterType::Peaking:
    default:
        m_B0 = 1.0f + alpha;
        m_B1 = -2.0f * cosOmega;
        m_B2 = 1.0f - alpha;
        a0 = 1.0f + alpha;
        m_A1 = -2.0f * cosOmega;
        m_A2 = 1.0f - alpha;
        break;
    }

    // Normalize coefficients by a0
    float invA0 = 1.0f / a0;
    m_B0 *= invA0;
    m_B1 *= invA0;
    m_B2 *= invA0;
    m_A1 *= invA0;
    m_A2 *= invA0;
}

float BiquadFilter::ProcessSample(float inSample)
{
    float outSample = m_B0 * inSample + m_B1 * m_X1 + m_B2 * m_X2 - m_A1 * m_Y1 - m_A2 * m_Y2;

    m_X2 = m_X1;
    m_X1 = inSample;
    m_Y2 = m_Y1;
    m_Y1 = outSample;

    return outSample;
}

void BiquadFilter::ProcessBlock(const AudioBufferBlock &inBlock, AudioBufferBlock &outBlock)
{
    for (uint32_t i = 0; i < AUDIO_BLOCK_SIZE; ++i)
    {
        outBlock.Samples[i] = ProcessSample(inBlock.Samples[i]);
    }
}

void BiquadFilter::Reset()
{
    m_X1 = m_X2 = 0.0f;
    m_Y1 = m_Y2 = 0.0f;
}

} // namespace SoundStudio
