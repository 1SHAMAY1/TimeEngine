#pragma once

#include <functional>
#include <unordered_map>
#include <string>

namespace TE {

    class Timer {
    public:
        using Callback = std::function<void()>;

        struct TimerRef {
            std::string id;
        };

        static TimerRef Set(float duration, Callback callback, bool loop = false);
        static TimerRef SetFrames(int frameCount, Callback callback, bool loop = false);
        static TimerRef NextFrame(Callback callback);

        static void Clear(const TimerRef& ref);
        static void Update(float deltaTime);
        static void Shutdown();

    private:
        struct TimerData {
            Callback callback;
            float remaining = 0.0f;
            float original = 0.0f;
            bool loop = false;
            bool frameMode = false;
            int framesRemaining = 0;
        };

        inline static std::unordered_map<std::string, TimerData> s_Timers;
        inline static int s_Counter = 0;
    };

}