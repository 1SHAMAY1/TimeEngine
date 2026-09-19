#pragma once

#include "../../DSP/ADSREnvelope.hpp"
#include "../../DSP/BiquadFilter.hpp"
#include "../../DSP/DelayLine.hpp"
#include "../../DSP/FreeverbReverb.hpp"
#include "../../DSP/WaveTableOscillator.hpp"
#include "../SoundGraphNode.hpp"

namespace SoundStudio
{

// 1. Oscillator Node
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

// 2. ADSR Envelope Node
class ADSREnvelopeNode : public SoundGraphNode
{
public:
    ADSREnvelopeNode();
    TEString GetNodeTypeName() const override { return "ADSR Envelope"; }
    TEString GetCategory() const override { return "Envelopes"; }
    void ExecuteDSP(const AudioDSPContext &ctx) override;
    void ResetState() override { m_Env.Reset(); }

    void TriggerAttack() { m_Env.TriggerAttack(); }
    void TriggerRelease() { m_Env.TriggerRelease(); }

private:
    ADSREnvelope m_Env;
};

// 3. Biquad Filter Node
class FilterNode : public SoundGraphNode
{
public:
    FilterNode();
    TEString GetNodeTypeName() const override { return "Filter"; }
    TEString GetCategory() const override { return "Filters"; }
    void ExecuteDSP(const AudioDSPContext &ctx) override;
    void ResetState() override { m_Filter.Reset(); }

    void SetFilterType(EFilterType type) { m_FilterType = type; }
    EFilterType GetFilterType() const { return m_FilterType; }

private:
    BiquadFilter m_Filter;
    EFilterType m_FilterType = EFilterType::LowPass12;
};

// 4. Delay Effect Node
class DelayNode : public SoundGraphNode
{
public:
    DelayNode();
    TEString GetNodeTypeName() const override { return "Delay"; }
    TEString GetCategory() const override { return "Effects"; }
    void ExecuteDSP(const AudioDSPContext &ctx) override;
    void ResetState() override { m_Delay.Reset(); }

private:
    DelayLine m_Delay;
};

// 5. Reverb Effect Node
class ReverbNode : public SoundGraphNode
{
public:
    ReverbNode();
    TEString GetNodeTypeName() const override { return "Reverb"; }
    TEString GetCategory() const override { return "Effects"; }
    void ExecuteDSP(const AudioDSPContext &ctx) override;
    void ResetState() override { m_Reverb.Reset(); }

private:
    FreeverbReverb m_Reverb;
};

// 6. Gain / Multiplier Node
class GainNode : public SoundGraphNode
{
public:
    GainNode();
    TEString GetNodeTypeName() const override { return "Gain"; }
    TEString GetCategory() const override { return "Math"; }
    void ExecuteDSP(const AudioDSPContext &ctx) override;
};

// 7. Audio Mixer Node (2-to-1)
class MixerNode : public SoundGraphNode
{
public:
    MixerNode();
    TEString GetNodeTypeName() const override { return "Mixer"; }
    TEString GetCategory() const override { return "Routing"; }
    void ExecuteDSP(const AudioDSPContext &ctx) override;
};

// 8. BPM Trigger Clock Node
class BPMClockNode : public SoundGraphNode
{
public:
    BPMClockNode();
    TEString GetNodeTypeName() const override { return "BPM Clock"; }
    TEString GetCategory() const override { return "Control"; }
    void ExecuteDSP(const AudioDSPContext &ctx) override;
    void ResetState() override { m_Accumulator = 0.0f; }

private:
    float m_Accumulator = 0.0f;
};

// 9. Master Audio Output Node (Terminal DAC)
class AudioOutputNode : public SoundGraphNode
{
public:
    AudioOutputNode();
    TEString GetNodeTypeName() const override { return "Audio Output"; }
    TEString GetCategory() const override { return "Output"; }
    void ExecuteDSP(const AudioDSPContext &ctx) override;

    const AudioBufferBlock &GetLeftOutput() const { return m_LeftBlock; }
    const AudioBufferBlock &GetRightOutput() const { return m_RightBlock; }

private:
    AudioBufferBlock m_LeftBlock;
    AudioBufferBlock m_RightBlock;
};

} // namespace SoundStudio
