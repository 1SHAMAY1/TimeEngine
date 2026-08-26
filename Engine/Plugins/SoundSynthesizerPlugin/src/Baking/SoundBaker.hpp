#pragma once

#include "../Graph/SoundGraphEvaluator.hpp"
#include "Utils/TEString.hpp"

namespace SoundStudio
{

struct SoundBakeOptions
{
    TEString OutputPath = "Assets/Sounds/BakedSound.wav";
    float DurationSeconds = 2.0f;
    uint32_t SampleRate = 44100;
    uint32_t Channels = 2;
    int TriggerMidiNote = 69; // A4 (440Hz)
    float TriggerDurationSeconds = 1.0f;
};

class SoundBaker
{
public:
    static bool BakeGraphToWAV(TERef<SoundGraph> graph, const SoundBakeOptions &options);
};

} // namespace SoundStudio
