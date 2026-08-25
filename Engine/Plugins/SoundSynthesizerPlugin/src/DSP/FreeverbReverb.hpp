#pragma once

#include "GameFrameWork/GameplayUtils.hpp"
#include "../SoundStudioTypes.hpp"
#include <algorithm>
#include <vector>

namespace SoundStudio {

struct CombFilter
{
    TEArray<float> Buffer;
    uint32_t BufferIndex = 0;
    float FilterStore = 0.0f;
    float Feedback = 0.5f;
    float Damp = 0.5f;

    void SetSize(uint32_t size)
    {
        Buffer.resize(size, 0.0f);
        BufferIndex = 0;
        FilterStore = 0.0f;
    }

    float Process(float input)
    {
        float output = Buffer[BufferIndex];
        FilterStore = (output * (1.0f - Damp)) + (FilterStore * Damp);
        Buffer[BufferIndex] = input + (FilterStore * Feedback);
        if (++BufferIndex >= Buffer.size())
            BufferIndex = 0;
        return output;
    }

    void Mute()
    {
        std::fill(Buffer.begin(), Buffer.end(), 0.0f);
        FilterStore = 0.0f;
    }
};

struct AllPassFilter
{
    TEArray<float> Buffer;
    uint32_t BufferIndex = 0;
    float Feedback = 0.5f;

    void SetSize(uint32_t size)
    {
        Buffer.resize(size, 0.0f);
        BufferIndex = 0;
    }

    float Process(float input)
    {
        float bufout = Buffer[BufferIndex];
        float output = -input + bufout;
        Buffer[BufferIndex] = input + (bufout * Feedback);
        if (++BufferIndex >= Buffer.size())
            BufferIndex = 0;
        return output;
    }

    void Mute()
    {
        std::fill(Buffer.begin(), Buffer.end(), 0.0f);
    }
};

class FreeverbReverb
{
public:
    FreeverbReverb();

    void SetParameters(float roomSize, float damping, float wetLevel, float dryLevel, float width);
    void ProcessBlock(const StereoAudioBlock &inBlock, StereoAudioBlock &outBlock);
    void Reset();

private:
    static constexpr int NUM_COMBS = 8;
    static constexpr int NUM_ALLPASSES = 4;

    CombFilter m_CombsL[NUM_COMBS];
    CombFilter m_CombsR[NUM_COMBS];

    AllPassFilter m_AllPassL[NUM_ALLPASSES];
    AllPassFilter m_AllPassR[NUM_ALLPASSES];

    float m_RoomSize = 0.5f;
    float m_Damping = 0.5f;
    float m_Wet = 0.33f;
    float m_Dry = 0.67f;
    float m_Width = 1.0f;
};

} // namespace SoundStudio
