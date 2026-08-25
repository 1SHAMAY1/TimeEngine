#pragma once

#include "../SoundGraphNode.hpp"

namespace SoundStudio {

class GainNode : public SoundGraphNode
{
public:
    GainNode();
    TEString GetNodeTypeName() const override { return "Gain"; }
    TEString GetCategory() const override { return "Math"; }
    void ExecuteDSP(const AudioDSPContext &ctx) override;
};

class MixerNode : public SoundGraphNode
{
public:
    MixerNode();
    TEString GetNodeTypeName() const override { return "Mixer"; }
    TEString GetCategory() const override { return "Routing"; }
    void ExecuteDSP(const AudioDSPContext &ctx) override;
};

class BPMClockNode : public SoundGraphNode
{
public:
    BPMClockNode();
    TEString GetNodeTypeName() const override { return "BPM Clock"; }
    TEString GetCategory() const override { return "Control"; }
    void ExecuteDSP(const AudioDSPContext &ctx) override;

private:
    float m_Accumulator = 0.0f;
};

} // namespace SoundStudio
