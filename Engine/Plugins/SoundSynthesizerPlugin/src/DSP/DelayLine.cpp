#include "DelayLine.hpp"
#include "AudioDSPUtils.hpp"
#include "GameFrameWork/GameplayUtils.hpp"
#include <algorithm>
#include <cmath>

namespace SoundStudio
{

DelayLine::DelayLine(uint32_t maxDelaySamples) { m_Buffer.resize(maxDelaySamples, 0.0f); }

void DelayLine::SetParameters(float delayTimeMs, float feedback, float dryWet, float sampleRate)
{
    m_SampleRate = sampleRate;
    m_Feedback = AudioDSPUtils::Clamp(feedback, 0.0f, 0.95f);
    m_DryWet = AudioDSPUtils::Clamp(dryWet, 0.0f, 1.0f);

    float delaySeconds = delayTimeMs * 0.001f;
    m_DelaySamples = (uint32_t)(delaySeconds * sampleRate);
    if (m_DelaySamples >= m_Buffer.size())
    {
        m_DelaySamples = (uint32_t)m_Buffer.size() - 1;
    }
    if (m_DelaySamples < 1)
    {
        m_DelaySamples = 1;
    }
}

float DelayLine::ProcessSample(float inSample)
{
    uint32_t bufferSize = (uint32_t)m_Buffer.size();
    uint32_t readIndex = (m_WriteIndex + bufferSize - m_DelaySamples) % bufferSize;

    float delayedSample = m_Buffer[readIndex];
    m_Buffer[m_WriteIndex] = inSample + delayedSample * m_Feedback;

    m_WriteIndex = (m_WriteIndex + 1) % bufferSize;

    return inSample * (1.0f - m_DryWet) + delayedSample * m_DryWet;
}

void DelayLine::ProcessBlock(const AudioBufferBlock &inBlock, AudioBufferBlock &outBlock)
{
    for (uint32_t i = 0; i < AUDIO_BLOCK_SIZE; ++i)
    {
        outBlock.Samples[i] = ProcessSample(inBlock.Samples[i]);
    }
}

void DelayLine::Reset()
{
    std::fill(m_Buffer.begin(), m_Buffer.end(), 0.0f);
    m_WriteIndex = 0;
}

} // namespace SoundStudio
