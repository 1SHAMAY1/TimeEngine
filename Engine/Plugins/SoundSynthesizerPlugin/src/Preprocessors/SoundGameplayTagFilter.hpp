#pragma once

#include "Utils/TEString.hpp"

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
