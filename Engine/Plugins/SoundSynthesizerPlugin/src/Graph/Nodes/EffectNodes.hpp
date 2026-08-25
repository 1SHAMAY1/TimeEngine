#pragma once

#include "../../DSP/DelayLine.hpp"
#include "../../DSP/FreeverbReverb.hpp"
#include "../SoundGraphNode.hpp"

namespace SoundStudio {

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

} // namespace SoundStudio
