#include "Core/Time/Timer.hpp"

namespace TE {

    Timer::TimerRef Timer::Set(float duration, Callback callback, bool loop) {
        std::string id = "timer_" + std::to_string(++s_Counter);
        s_Timers[id] = { callback, duration, duration, loop, false, 0 };
        return { id };
    }

    Timer::TimerRef Timer::SetFrames(int frameCount, Callback callback, bool loop) {
        std::string id = "timer_" + std::to_string(++s_Counter);
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
        std::vector<std::string> toRemove;

        for (auto& [id, data] : s_Timers) {
            if (data.frameMode) {
                data.framesRemaining--;
                if (data.framesRemaining <= 0) {
                    data.callback();
                    if (data.loop)
                        data.framesRemaining = static_cast<int>(data.original);
                    else
                        toRemove.push_back(id);
                }
            } else {
                data.remaining -= deltaTime;
                if (data.remaining <= 0.0f) {
                    data.callback();
                    if (data.loop)
                        data.remaining = data.original;
                    else
                        toRemove.push_back(id);
                }
            }
        }

        for (const auto& id : toRemove)
            s_Timers.erase(id);
    }

    void Timer::Shutdown() {
        s_Timers.clear();
    }

}