#include "EnvelopeNode.hpp"

namespace SoundStudio
{

ADSREnvelopeNode::ADSREnvelopeNode()
{
    AddInputPin("Attack (s)", EAudioPinType::Float, 0.05f);
    AddInputPin("Decay (s)", EAudioPinType::Float, 0.1f);
    AddInputPin("Sustain", EAudioPinType::Float, 0.7f);
    AddInputPin("Release (s)", EAudioPinType::Float, 0.3f);
    AddInputPin("Trigger In", EAudioPinType::Trigger, 0.0f);
    AddOutputPin("Envelope Out", EAudioPinType::AudioSignal, 0.0f);
}

void ADSREnvelopeNode::ExecuteDSP(const AudioDSPContext &ctx)
{
    m_Env.SetAttackTime(GetInputValueFloat("Attack (s)", ctx));
    m_Env.SetDecayTime(GetInputValueFloat("Decay (s)", ctx));
    m_Env.SetSustainLevel(GetInputValueFloat("Sustain", ctx));
    m_Env.SetReleaseTime(GetInputValueFloat("Release (s)", ctx));

    if (IsInputTriggered("Trigger In"))
    {
        m_Env.TriggerAttack();
    }

    auto &outBlock = GetOutputBuffer("Envelope Out");
    m_Env.ProcessBlock(ctx.DeltaTime, outBlock);
}

} // namespace SoundStudio
