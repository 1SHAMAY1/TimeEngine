#include "TriggerSequencerNode.hpp"
#include <algorithm>

namespace SoundStudio
{

TriggerSequencerNode::TriggerSequencerNode()
{
    AddInputPin("BPM", EAudioPinType::Float, 120.0f);
    AddInputPin("Division", EAudioPinType::Float, 1.0f); // 1 = Quarter, 2 = Eighth, 4 = Sixteenth
    AddOutputPin("Trigger Out", EAudioPinType::Trigger, 0.0f);
}

void TriggerSequencerNode::ExecuteDSP(const AudioDSPContext &ctx)
{
    float bpm = std::max(10.0f, GetInputValueFloat("BPM", ctx));
    float div = std::max(0.25f, GetInputValueFloat("Division", ctx));
    float beatPeriod = (60.0f / bpm) / div;

    m_Accumulator += (float)AUDIO_BLOCK_SIZE * ctx.DeltaTime;
    if (m_Accumulator >= beatPeriod)
    {
        m_Accumulator -= beatPeriod;
        FireOutputTrigger("Trigger Out");
    }
}

} // namespace SoundStudio
