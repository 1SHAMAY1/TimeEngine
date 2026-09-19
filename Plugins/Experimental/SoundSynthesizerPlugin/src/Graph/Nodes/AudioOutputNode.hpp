#pragma once

#include "../SoundGraphNode.hpp"

namespace SoundStudio
{

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
