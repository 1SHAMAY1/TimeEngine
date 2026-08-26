#include "FreeverbReverb.hpp"
#include "AudioDSPUtils.hpp"

namespace SoundStudio
{

static const int s_CombTuningsL[8] = {1116, 1188, 1277, 1356, 1422, 1491, 1557, 1617};
static const int s_CombTuningsR[8] = {1116 + 23, 1188 + 23, 1277 + 23, 1356 + 23,
                                      1422 + 23, 1491 + 23, 1557 + 23, 1617 + 23};
static const int s_AllPassTuningsL[4] = {556, 441, 341, 225};
static const int s_AllPassTuningsR[4] = {556 + 23, 441 + 23, 341 + 23, 225 + 23};

FreeverbReverb::FreeverbReverb()
{
    for (int i = 0; i < NUM_COMBS; ++i)
    {
        m_CombsL[i].SetSize(s_CombTuningsL[i]);
        m_CombsR[i].SetSize(s_CombTuningsR[i]);
    }

    for (int i = 0; i < NUM_ALLPASSES; ++i)
    {
        m_AllPassL[i].SetSize(s_AllPassTuningsL[i]);
        m_AllPassR[i].SetSize(s_AllPassTuningsR[i]);
        m_AllPassL[i].Feedback = 0.5f;
        m_AllPassR[i].Feedback = 0.5f;
    }

    SetParameters(0.5f, 0.5f, 0.33f, 0.67f, 1.0f);
}

void FreeverbReverb::SetParameters(float roomSize, float damping, float wetLevel, float dryLevel, float width)
{
    m_RoomSize = AudioDSPUtils::Clamp(roomSize, 0.0f, 0.98f);
    m_Damping = AudioDSPUtils::Clamp(damping, 0.0f, 1.0f);
    m_Wet = AudioDSPUtils::Clamp(wetLevel, 0.0f, 1.0f);
    m_Dry = AudioDSPUtils::Clamp(dryLevel, 0.0f, 1.0f);
    m_Width = AudioDSPUtils::Clamp(width, 0.0f, 1.0f);

    for (int i = 0; i < NUM_COMBS; ++i)
    {
        m_CombsL[i].Feedback = m_RoomSize;
        m_CombsR[i].Feedback = m_RoomSize;
        m_CombsL[i].Damp = m_Damping;
        m_CombsR[i].Damp = m_Damping;
    }
}

void FreeverbReverb::ProcessBlock(const StereoAudioBlock &inBlock, StereoAudioBlock &outBlock)
{
    float wet1 = m_Wet * (m_Width / 2.0f + 0.5f);
    float wet2 = m_Wet * ((1.0f - m_Width) / 2.0f);

    for (uint32_t i = 0; i < AUDIO_BLOCK_SIZE; ++i)
    {
        float inL = inBlock.Left.Samples[i];
        float inR = inBlock.Right.Samples[i];
        float input = (inL + inR) * 0.015f; // Scale input

        float outL = 0.0f;
        float outR = 0.0f;

        // Accumulate Comb Filters
        for (int c = 0; c < NUM_COMBS; ++c)
        {
            outL += m_CombsL[c].Process(input);
            outR += m_CombsR[c].Process(input);
        }

        // Pass through AllPass Filters
        for (int a = 0; a < NUM_ALLPASSES; ++a)
        {
            outL = m_AllPassL[a].Process(outL);
            outR = m_AllPassR[a].Process(outR);
        }

        // Mix wet and dry
        outBlock.Left.Samples[i] = outL * wet1 + outR * wet2 + inL * m_Dry;
        outBlock.Right.Samples[i] = outR * wet1 + outL * wet2 + inR * m_Dry;
    }
}

void FreeverbReverb::Reset()
{
    for (int i = 0; i < NUM_COMBS; ++i)
    {
        m_CombsL[i].Mute();
        m_CombsR[i].Mute();
    }
    for (int i = 0; i < NUM_ALLPASSES; ++i)
    {
        m_AllPassL[i].Mute();
        m_AllPassR[i].Mute();
    }
}

} // namespace SoundStudio
