#pragma once

#include "GameFrameWork/GameplayUtils.hpp"
#include "../SoundStudioTypes.hpp"
#include <vector>

namespace SoundStudio {

class DelayLine
{
public:
    DelayLine(uint32_t maxDelaySamples = 44100 * 2);

    void SetParameters(float delayTimeMs, float feedback, float dryWet, float sampleRate);
    float ProcessSample(float inSample);
    void ProcessBlock(const AudioBufferBlock &inBlock, AudioBufferBlock &outBlock);
    void Reset();

private:
    TEArray<float> m_Buffer;
    uint32_t m_WriteIndex = 0;
    uint32_t m_DelaySamples = 4410;
    float m_Feedback = 0.4f;
    float m_DryWet = 0.3f;
    float m_SampleRate = 44100.0f;
};

} // namespace SoundStudio
