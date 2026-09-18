#pragma once

#include "EngineTypes/TEString.hpp"

class TTSGameplayTagFilter
{
public:
    static bool ShouldSpeak();
    static bool IsTTSMuted();
};
