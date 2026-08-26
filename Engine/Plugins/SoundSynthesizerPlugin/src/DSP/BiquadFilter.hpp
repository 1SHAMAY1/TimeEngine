#pragma once

#include "../SoundStudioTypes.hpp"
#include "AudioDSPContext.hpp"

namespace SoundStudio
{

class BiquadFilter
{
public:
    BiquadFilter() = default;

    void SetParameters(EFilterType type, float cutoffHz, float q, float sampleRate);
    float ProcessSample(float inSample);
    void ProcessBlock(const AudioBufferBlock &inBlock, AudioBufferBlock &outBlock);
    void Reset();

private:
    float m_B0 = 1.0f, m_B1 = 0.0f, m_B2 = 0.0f;
    float m_A1 = 0.0f, m_A2 = 0.0f;
    float m_X1 = 0.0f, m_X2 = 0.0f;
    float m_Y1 = 0.0f, m_Y2 = 0.0f;
};

} // namespace SoundStudio
