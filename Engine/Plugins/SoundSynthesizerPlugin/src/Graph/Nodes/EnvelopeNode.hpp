#pragma once

#include "../../DSP/ADSREnvelope.hpp"
#include "../SoundGraphNode.hpp"

namespace SoundStudio
{

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

using EnvelopeNode = ADSREnvelopeNode;

} // namespace SoundStudio
