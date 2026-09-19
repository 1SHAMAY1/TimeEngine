#include "MathNodes.hpp"

namespace SoundStudio
{

GainNode::GainNode()
{
    AddInputPin("Audio In", EAudioPinType::AudioSignal, 0.0f);
    AddInputPin("Gain", EAudioPinType::Float, 1.0f);
    AddInputPin("Mod In", EAudioPinType::AudioSignal, 1.0f);
    AddOutputPin("Audio Out", EAudioPinType::AudioSignal, 0.0f);
}

void GainNode::ExecuteDSP(const AudioDSPContext &ctx)
{
    float staticGain = GetInputValueFloat("Gain", ctx);
    const auto &inBlock = GetInputBuffer("Audio In");
    const auto &modBlock = GetInputBuffer("Mod In");
    auto &outBlock = GetOutputBuffer("Audio Out");

    for (uint32_t i = 0; i < AUDIO_BLOCK_SIZE; ++i)
    {
        outBlock.Samples[i] = inBlock.Samples[i] * staticGain * modBlock.Samples[i];
    }
}

MixerNode::MixerNode()
{
    AddInputPin("Input A", EAudioPinType::AudioSignal, 0.0f);
    AddInputPin("Input B", EAudioPinType::AudioSignal, 0.0f);
    AddInputPin("Gain A", EAudioPinType::Float, 0.5f);
    AddInputPin("Gain B", EAudioPinType::Float, 0.5f);
    AddOutputPin("Audio Out", EAudioPinType::AudioSignal, 0.0f);
}

void MixerNode::ExecuteDSP(const AudioDSPContext &ctx)
{
    float gA = GetInputValueFloat("Gain A", ctx);
    float gB = GetInputValueFloat("Gain B", ctx);

    const auto &inA = GetInputBuffer("Input A");
    const auto &inB = GetInputBuffer("Input B");
    auto &outBlock = GetOutputBuffer("Audio Out");

    for (uint32_t i = 0; i < AUDIO_BLOCK_SIZE; ++i)
    {
        outBlock.Samples[i] = inA.Samples[i] * gA + inB.Samples[i] * gB;
    }
}

BPMClockNode::BPMClockNode()
{
    AddInputPin("BPM", EAudioPinType::Float, 120.0f);
    AddOutputPin("Trigger Out", EAudioPinType::Trigger, 0.0f);
}

void BPMClockNode::ExecuteDSP(const AudioDSPContext &ctx)
{
    float bpm = std::max(10.0f, GetInputValueFloat("BPM", ctx));
    float beatPeriod = 60.0f / bpm;

    m_Accumulator += (float)AUDIO_BLOCK_SIZE * ctx.DeltaTime;
    if (m_Accumulator >= beatPeriod)
    {
        m_Accumulator -= beatPeriod;
        FireOutputTrigger("Trigger Out");
    }
}

} // namespace SoundStudio
