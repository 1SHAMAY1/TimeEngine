#pragma once

#include "Core/PreRequisites.h"
#include "GameFrameWork/GameplayUtils.hpp"
#include <functional>

class Timer
{
public:
    using Callback = std::function<void()>;

    struct TimerRef
    {
        TEString id;
    };

    static TimerRef Set(float duration, Callback callback, bool loop = false);
    static TimerRef SetFrames(int frameCount, Callback callback, bool loop = false);
    static TimerRef NextFrame(Callback callback);

    static void Clear(const TimerRef &ref);
    static void Update(float deltaTime);
    static void Shutdown();

private:
    struct TimerData
    {
        Callback callback;
        float remaining = 0.0f;
        float original = 0.0f;
        bool loop = false;
        bool frameMode = false;
        int framesRemaining = 0;
    };

    inline static TEMap<TEString, TimerData> s_Timers;
    inline static int s_Counter = 0;
};
