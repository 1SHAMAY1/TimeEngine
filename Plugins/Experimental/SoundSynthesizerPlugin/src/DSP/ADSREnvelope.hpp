#pragma once

#include "../SoundStudioTypes.hpp"
#include "AudioDSPContext.hpp"

namespace SoundStudio
{

enum class EEnvelopeState
{
    Idle = 0,
    Attack,
    Decay,
    Sustain,
    Release
};

class ADSREnvelope
{
public:
    ADSREnvelope() = default;

    void SetAttackTime(float seconds) { m_AttackTime = std::max(0.001f, seconds); }
    void SetDecayTime(float seconds) { m_DecayTime = std::max(0.001f, seconds); }
    void SetSustainLevel(float level) { m_SustainLevel = std::max(0.0f, std::min(1.0f, level)); }
    void SetReleaseTime(float seconds) { m_ReleaseTime = std::max(0.001f, seconds); }

    void TriggerAttack();
    void TriggerRelease();
    void Reset();

    float ProcessSample(float dt);
    void ProcessBlock(float dt, AudioBufferBlock &outBlock);

    bool IsActive() const { return m_State != EEnvelopeState::Idle; }
    EEnvelopeState GetState() const { return m_State; }

private:
    float m_AttackTime = 0.05f;
    float m_DecayTime = 0.1f;
    float m_SustainLevel = 0.7f;
    float m_ReleaseTime = 0.3f;

    EEnvelopeState m_State = EEnvelopeState::Idle;
    float m_CurrentLevel = 0.0f;
};

} // namespace SoundStudio
