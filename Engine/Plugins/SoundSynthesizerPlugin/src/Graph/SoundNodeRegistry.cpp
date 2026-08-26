#include "SoundNodeRegistry.hpp"

namespace SoundStudio
{

static TEMap<TEString, SoundNodeInfo> s_Registry;

void SoundNodeRegistry::RegisterNode(const TEString &typeName, const TEString &category,
                                     std::function<TERef<SoundGraphNode>()> factory)
{
    s_Registry[typeName] = {typeName, category, factory};
}

TERef<SoundGraphNode> SoundNodeRegistry::CreateNode(const TEString &typeName)
{
    auto it = s_Registry.find(typeName);
    if (it != s_Registry.end() && it->second.Factory)
    {
        return it->second.Factory();
    }
    return nullptr;
}

const TEMap<TEString, SoundNodeInfo> &SoundNodeRegistry::GetRegisteredNodes() { return s_Registry; }

void SoundNodeRegistry::InitializeDefaults()
{
    if (!s_Registry.empty())
        return;

    RegisterNode("Oscillator", "Generators", []() { return CreateRef<OscillatorNode>(); });
    RegisterNode("ADSR Envelope", "Envelopes", []() { return CreateRef<ADSREnvelopeNode>(); });
    RegisterNode("Filter", "Filters", []() { return CreateRef<FilterNode>(); });
    RegisterNode("Delay", "Effects", []() { return CreateRef<DelayNode>(); });
    RegisterNode("Reverb", "Effects", []() { return CreateRef<ReverbNode>(); });
    RegisterNode("Gain", "Math", []() { return CreateRef<GainNode>(); });
    RegisterNode("Mixer", "Routing", []() { return CreateRef<MixerNode>(); });
    RegisterNode("BPM Clock", "Control", []() { return CreateRef<BPMClockNode>(); });
    RegisterNode("Trigger Sequencer", "Control", []() { return CreateRef<TriggerSequencerNode>(); });
    RegisterNode("Wave Player", "Generators", []() { return CreateRef<WavePlayerNode>(); });
    RegisterNode("Audio Output", "Output", []() { return CreateRef<AudioOutputNode>(); });
}

} // namespace SoundStudio
