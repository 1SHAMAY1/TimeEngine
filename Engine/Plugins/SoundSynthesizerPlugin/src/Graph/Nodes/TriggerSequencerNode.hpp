#pragma once

#include "../SoundGraphNode.hpp"

namespace SoundStudio
{

class TriggerSequencerNode : public SoundGraphNode
{
public:
    TriggerSequencerNode();
    TEString GetNodeTypeName() const override { return "Trigger Sequencer"; }
    TEString GetCategory() const override { return "Control"; }
    void ExecuteDSP(const AudioDSPContext &ctx) override;
    void ResetState() override { m_Accumulator = 0.0f; }

private:
    float m_Accumulator = 0.0f;
};

} // namespace SoundStudio
