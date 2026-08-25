#pragma once

#include "../SoundStudioTypes.hpp"
#include <cstdint>

namespace SoundStudio {

struct AudioDSPContext
{
    uint32_t SampleRate = DEFAULT_SAMPLE_RATE;
    uint32_t BlockSize = AUDIO_BLOCK_SIZE;
    float DeltaTime = 1.0f / (float)DEFAULT_SAMPLE_RATE;
    float InvSampleRate = 1.0f / (float)DEFAULT_SAMPLE_RATE;
    float GlobalBPM = 120.0f;
    uint64_t CurrentSampleIndex = 0;
};

} // namespace SoundStudio
