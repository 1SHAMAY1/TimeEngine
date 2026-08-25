#include "SoundGraphNode.hpp"

namespace SoundStudio {

void SoundGraphNode::Initialize(uint64_t id)
{
    m_ID = id;
}

uint64_t SoundGraphNode::AddInputPin(const TEString &name, EAudioPinType type, float defaultVal)
{
    SoundPin pin;
    pin.ID = (m_ID << 16) | (m_NextPinId++);
    pin.NodeID = m_ID;
    pin.Name = name;
    pin.Type = type;
    pin.IsInput = true;
    pin.DefaultFloat = defaultVal;
    pin.Buffer.Fill(defaultVal);
    m_Inputs.push_back(pin);
    return pin.ID;
}

uint64_t SoundGraphNode::AddOutputPin(const TEString &name, EAudioPinType type, float defaultVal)
{
    SoundPin pin;
    pin.ID = (m_ID << 16) | (m_NextPinId++);
    pin.NodeID = m_ID;
    pin.Name = name;
    pin.Type = type;
    pin.IsInput = false;
    pin.DefaultFloat = defaultVal;
    pin.Buffer.Fill(defaultVal);
    m_Outputs.push_back(pin);
    return pin.ID;
}

SoundPin *SoundGraphNode::FindPin(uint64_t pinId)
{
    for (auto &p : m_Inputs)
    {
        if (p.ID == pinId)
            return &p;
    }
    for (auto &p : m_Outputs)
    {
        if (p.ID == pinId)
            return &p;
    }
    return nullptr;
}

SoundPin *SoundGraphNode::FindPinByName(const TEString &name, bool isInput)
{
    auto &list = isInput ? m_Inputs : m_Outputs;
    for (auto &p : list)
    {
        if (p.Name == name)
            return &p;
    }
    return nullptr;
}

float SoundGraphNode::GetInputValueFloat(const TEString &pinName, const AudioDSPContext &ctx, uint32_t sampleIdx)
{
    auto *pin = FindPinByName(pinName, true);
    if (!pin)
        return 0.0f;

    if (pin->Type == EAudioPinType::AudioSignal && sampleIdx < AUDIO_BLOCK_SIZE)
    {
        return pin->Buffer.Samples[sampleIdx];
    }
    return pin->Buffer.Samples[0];
}

const AudioBufferBlock &SoundGraphNode::GetInputBuffer(const TEString &pinName)
{
    static AudioBufferBlock s_Empty;
    auto *pin = FindPinByName(pinName, true);
    return pin ? pin->Buffer : s_Empty;
}

bool SoundGraphNode::IsInputTriggered(const TEString &pinName)
{
    auto *pin = FindPinByName(pinName, true);
    return pin ? pin->TriggerFired : false;
}

void SoundGraphNode::SetOutputValueFloat(const TEString &pinName, float val)
{
    auto *pin = FindPinByName(pinName, false);
    if (pin)
    {
        pin->Buffer.Fill(val);
    }
}

AudioBufferBlock &SoundGraphNode::GetOutputBuffer(const TEString &pinName)
{
    static AudioBufferBlock s_Empty;
    auto *pin = FindPinByName(pinName, false);
    return pin ? pin->Buffer : s_Empty;
}

void SoundGraphNode::FireOutputTrigger(const TEString &pinName)
{
    auto *pin = FindPinByName(pinName, false);
    if (pin)
    {
        pin->TriggerFired = true;
    }
}

} // namespace SoundStudio
