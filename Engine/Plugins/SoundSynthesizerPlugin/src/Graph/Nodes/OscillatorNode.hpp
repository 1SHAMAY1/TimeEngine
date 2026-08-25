#pragma once

#include "../../DSP/WaveTableOscillator.hpp"
#include "../SoundGraphNode.hpp"

namespace SoundStudio {

class OscillatorNode : public SoundGraphNode
{
public:
    OscillatorNode();
    TEString GetNodeTypeName() const override { return "Oscillator"; }
    TEString GetCategory() const override { return "Generators"; }
    void ExecuteDSP(const AudioDSPContext &ctx) override;
    void ResetState() override { m_Osc.Reset(); }

    void SetWaveform(EWaveformType type) { m_Waveform = type; }
    EWaveformType GetWaveform() const { return m_Waveform; }

private:
    WaveTableOscillator m_Osc;
    EWaveformType m_Waveform = EWaveformType::Sine;
};

} // namespace SoundStudio
