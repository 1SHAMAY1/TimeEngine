#include "Core/PreRequisites.h"
#include "Core/Time/Timer.hpp"


    Timer::TimerRef Timer::Set(float duration, Callback callback, bool loop) {
        TEString id = "timer_" + TEString::FromInt(++s_Counter);
        s_Timers[id] = { callback, duration, duration, loop, false, 0 };
        return { id };
    }

    Timer::TimerRef Timer::SetFrames(int frameCount, Callback callback, bool loop) {
        TEString id = "timer_" + TEString::FromInt(++s_Counter);
        s_Timers[id] = { callback, 0.0f, 0.0f, loop, true, frameCount };
        return { id };
    }

    Timer::TimerRef Timer::NextFrame(Callback callback) {
        return SetFrames(1, callback, false);
    }

    void Timer::Clear(const TimerRef& ref) {
        s_Timers.erase(ref.id);
    }

    void Timer::Update(float deltaTime) {
        TEArray<TEString> toRemove;

        for (auto& pair : s_Timers) {
            auto& id = pair.first;
            auto& data = pair.second;
            if (data.frameMode) {
                data.framesRemaining--;
                if (data.framesRemaining <= 0) {
                    data.callback();
                    if (data.loop)
                        data.framesRemaining = static_cast<int>(data.original);
                    else
                        toRemove.Add(id);
                }
            } else {
                data.remaining -= deltaTime;
                if (data.remaining <= 0.0f) {
                    data.callback();
                    if (data.loop)
                        data.remaining = data.original;
                    else
                        toRemove.Add(id);
                }
            }
        }

        for (size_t i = 0; i < toRemove.Num(); ++i)
            s_Timers.erase(toRemove[i]);
    }

    void Timer::Shutdown() {
        s_Timers.clear();
    }

