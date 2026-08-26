#include "ADSREnvelope.hpp"
#include <algorithm>

namespace SoundStudio
{

void ADSREnvelope::TriggerAttack() { m_State = EEnvelopeState::Attack; }

void ADSREnvelope::TriggerRelease()
{
    if (m_State != EEnvelopeState::Idle)
    {
        m_State = EEnvelopeState::Release;
    }
}

void ADSREnvelope::Reset()
{
    m_State = EEnvelopeState::Idle;
    m_CurrentLevel = 0.0f;
}

float ADSREnvelope::ProcessSample(float dt)
{
    switch (m_State)
    {
    case EEnvelopeState::Attack:
        m_CurrentLevel += dt / m_AttackTime;
        if (m_CurrentLevel >= 1.0f)
        {
            m_CurrentLevel = 1.0f;
            m_State = EEnvelopeState::Decay;
        }
        break;

    case EEnvelopeState::Decay:
        m_CurrentLevel -= dt / m_DecayTime;
        if (m_CurrentLevel <= m_SustainLevel)
        {
            m_CurrentLevel = m_SustainLevel;
            m_State = EEnvelopeState::Sustain;
        }
        break;

    case EEnvelopeState::Sustain:
        m_CurrentLevel = m_SustainLevel;
        break;

    case EEnvelopeState::Release:
        m_CurrentLevel -= dt / m_ReleaseTime;
        if (m_CurrentLevel <= 0.0f)
        {
            m_CurrentLevel = 0.0f;
            m_State = EEnvelopeState::Idle;
        }
        break;

    case EEnvelopeState::Idle:
    default:
        m_CurrentLevel = 0.0f;
        break;
    }

    return m_CurrentLevel;
}

void ADSREnvelope::ProcessBlock(float dt, AudioBufferBlock &outBlock)
{
    for (uint32_t i = 0; i < AUDIO_BLOCK_SIZE; ++i)
    {
        outBlock.Samples[i] = ProcessSample(dt);
    }
}

} // namespace SoundStudio
