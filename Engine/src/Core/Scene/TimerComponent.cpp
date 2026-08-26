#include "Core/PreRequisites.h"
#include "Core/Scene/TimerComponent.hpp"

void TimerComponent::AddTimer(const TEString &name, float interval, bool repeat)
{
    for (auto &timer : m_Timers)
    {
        if (timer.Name == name)
        {
            timer.Interval = interval;
            timer.Remaining = interval;
            timer.bRepeat = repeat;
            timer.bActive = true;
            return;
        }
    }

    TimerEntry entry;
    entry.Name = name;
    entry.Interval = interval;
    entry.Remaining = interval;
    entry.bRepeat = repeat;
    entry.bActive = true;
    m_Timers.Add(entry);
}

void TimerComponent::RemoveTimer(const TEString &name)
{
    for (int i = 0; i < (int)m_Timers.size(); ++i)
    {
        if (m_Timers[i].Name == name)
        {
            m_Timers.RemoveAt(i);
            break;
        }
    }
}

void TimerComponent::ResetTimer(const TEString &name)
{
    for (auto &timer : m_Timers)
    {
        if (timer.Name == name)
        {
            timer.Remaining = timer.Interval;
            timer.bActive = true;
            break;
        }
    }
}

void TimerComponent::PauseTimer(const TEString &name)
{
    for (auto &timer : m_Timers)
    {
        if (timer.Name == name)
        {
            timer.bActive = false;
            break;
        }
    }
}

void TimerComponent::ResumeTimer(const TEString &name)
{
    for (auto &timer : m_Timers)
    {
        if (timer.Name == name)
        {
            timer.bActive = true;
            break;
        }
    }
}

bool TimerComponent::HasTimer(const TEString &name) const
{
    for (const auto &timer : m_Timers)
    {
        if (timer.Name == name)
            return true;
    }
    return false;
}

void TimerComponent::Tick(float deltaTime)
{
    for (int i = 0; i < (int)m_Timers.size(); ++i)
    {
        auto &timer = m_Timers[i];
        if (!timer.bActive)
            continue;

        timer.Remaining -= deltaTime;
        if (timer.Remaining <= 0.0f)
        {
            DispatchScriptTimer(timer.Name);

            if (timer.bRepeat)
            {
                timer.Remaining += timer.Interval;
                if (timer.Remaining <= 0.0f)
                    timer.Remaining = timer.Interval;
            }
            else
            {
                m_Timers.RemoveAt(i);
                --i;
            }
        }
    }
}
