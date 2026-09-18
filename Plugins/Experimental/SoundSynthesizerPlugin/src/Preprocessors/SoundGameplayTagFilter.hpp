#pragma once

#include "EngineTypes/TEString.hpp"

namespace SoundStudio
{

class SoundGameplayTagFilter
{
public:
    static bool ShouldSynthesize();
    static bool IsSynthMuted();
    static float GetTagVolumeMultiplier();
};

} // namespace SoundStudio
