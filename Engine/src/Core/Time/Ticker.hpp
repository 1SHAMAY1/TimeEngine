#pragma once

#include <functional>

namespace TE {

    class Ticker {
    public:
        using Callback = std::function<void(float& deltaTime, float totalTime)>;

        Ticker(Callback callback, float rate = 0.0f);
        ~Ticker();

        void Update(float deltaTime);
        void Reset();
        void SetRate(float rate);
        void SetCallback(Callback callback);

        float GetRate() const;
        float GetElapsed() const;

    private:
        Callback m_Callback;
        float m_Rate = 0.0f;
        float m_Elapsed = 0.0f;
        float m_Total = 0.0f;
    };

}