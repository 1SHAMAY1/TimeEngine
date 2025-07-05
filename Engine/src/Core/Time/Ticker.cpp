#include "Core/Time/Ticker.hpp"

namespace TE {

    Ticker::Ticker(Callback callback, float rate)
        : m_Callback(callback), m_Rate(rate) {}

    Ticker::~Ticker() {}

    void Ticker::Update(float deltaTime) {
        m_Elapsed += deltaTime;
        m_Total += deltaTime;

        if (m_Rate == 0.0f || m_Elapsed >= m_Rate) {
            if (m_Callback)
                m_Callback(deltaTime, m_Total);

            if (m_Rate != 0.0f)
                m_Elapsed = 0.0f;
        }
    }

    void Ticker::Reset() {
        m_Elapsed = 0.0f;
        m_Total = 0.0f;
    }

    void Ticker::SetRate(float rate) { m_Rate = rate; }
    void Ticker::SetCallback(Callback callback) { m_Callback = callback; }

    float Ticker::GetRate() const { return m_Rate; }
    float Ticker::GetElapsed() const { return m_Elapsed; }

}