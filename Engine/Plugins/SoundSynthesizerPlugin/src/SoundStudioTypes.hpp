#pragma once

#include "Core/PreRequisites.h"
#include "Utils/MathUtils.hpp"
#include "Utils/TEString.hpp"
#include <vector>

namespace SoundStudio
{

static constexpr uint32_t AUDIO_BLOCK_SIZE = 128; // 128 samples per DSP evaluation block
static constexpr uint32_t DEFAULT_SAMPLE_RATE = 44100;

enum class EWaveformType
{
    Sine = 0,
    Sawtooth,
    Square,
    Triangle,
    WhiteNoise,
    PinkNoise
};

enum class EFilterType
{
    LowPass12 = 0,
    LowPass24,
    HighPass,
    BandPass,
    Notch,
    Peaking
};

enum class EAudioPinType
{
    AudioSignal = 0, // Continuous audio buffer (Audio Rate)
    Trigger,         // Discrete pulse/event (Control Rate)
    Float,           // Single numeric modulation value
    Int,             // Integer value
    Enum             // Enumeration / selection
};

// Represents a 128-sample single-channel block of audio
struct AudioBufferBlock
{
    float Samples[AUDIO_BLOCK_SIZE] = {0.0f};

    void Clear()
    {
        for (uint32_t i = 0; i < AUDIO_BLOCK_SIZE; ++i)
            Samples[i] = 0.0f;
    }

    void Fill(float value)
    {
        for (uint32_t i = 0; i < AUDIO_BLOCK_SIZE; ++i)
            Samples[i] = value;
    }
};

// Stereo audio block pair
struct StereoAudioBlock
{
    AudioBufferBlock Left;
    AudioBufferBlock Right;

    void Clear()
    {
        Left.Clear();
        Right.Clear();
    }
};

} // namespace SoundStudio
