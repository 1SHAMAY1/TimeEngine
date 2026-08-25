#pragma once

#include "../SoundGraphNode.hpp"
#include <vector>

namespace SoundStudio {

class WavePlayerNode : public SoundGraphNode
{
public:
    WavePlayerNode();
    TEString GetNodeTypeName() const override { return "Wave Player"; }
    TEString GetCategory() const override { return "Generators"; }
    void ExecuteDSP(const AudioDSPContext &ctx) override;
    void ResetState() override { m_Playhead = 0.0f; m_IsPlaying = false; }

    void LoadWaveFile(const TEString &path);

private:
    TEArray<float> m_SampleBuffer;
    float m_Playhead = 0.0f;
    bool m_IsPlaying = false;
};

} // namespace SoundStudio
