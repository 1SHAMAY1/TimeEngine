#pragma once

#include "GameFrameWork/TComponent.hpp"
#include "GameFrameWork/GameplayUtils.hpp"
#include "Core/Scene/ComponentRegistry.hpp"
#include "Utils/TEString.hpp"

struct TimerEntry
{
    TEString Name;
    float Interval = 1.0f;
    float Remaining = 1.0f;
    bool bRepeat = true;
    bool bActive = true;
};

TE_CLASS()
class TE_API TimerComponent : public TComponent
{
public:
    TimerComponent() = default;
    virtual ~TimerComponent() override = default;

    void AddTimer(const TEString &name, float interval, bool repeat = true);
    void RemoveTimer(const TEString &name);
    void ResetTimer(const TEString &name);
    void PauseTimer(const TEString &name);
    void ResumeTimer(const TEString &name);
    bool HasTimer(const TEString &name) const;

    const TEArray<TimerEntry> &GetTimers() const { return m_Timers; }

    virtual void Tick(float deltaTime) override;

    inline static const TEString StaticClassName = "TimerComponent";
    virtual TEString GetClassName() const override { return StaticClassName; }

private:
    TEArray<TimerEntry> m_Timers;
};

#ifdef TE_EDITOR
T_REGISTER_COMPONENT(TimerComponent, "Timer Component")
T_REGISTER_PRESET(TimerPreset, "Timer Component", "Utility", [](EntityID id, EntityManager *em) {
    em->AddComponent<TimerComponent>(id);
})
#endif
