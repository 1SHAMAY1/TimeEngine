#pragma once

#include "../SoundStudioTypes.hpp"
#include "AudioDSPContext.hpp"

namespace SoundStudio {

class WaveTableOscillator
{
public:
    WaveTableOscillator() = default;

    void SetWaveform(EWaveformType type) { m_Type = type; }
    void SetFrequency(float freq) { m_Frequency = std::max(0.1f, freq); }
    void SetPulseWidth(float pw) { m_PulseWidth = std::max(0.01f, std::min(0.99f, pw)); }
    void SetPhase(float phase) { m_Phase = phase; }

    float ProcessSample(float sampleRate);
    void ProcessBlock(float sampleRate, AudioBufferBlock &outBlock);
    void Reset() { m_Phase = 0.0f; }

private:
    EWaveformType m_Type = EWaveformType::Sine;
    float m_Frequency = 440.0f;
    float m_PulseWidth = 0.5f;
    float m_Phase = 0.0f;

    // Pink noise generator state
    float m_B0 = 0.0f, m_B1 = 0.0f, m_B2 = 0.0f, m_B3 = 0.0f, m_B4 = 0.0f, m_B5 = 0.0f, m_B6 = 0.0f;
};

} // namespace SoundStudio
