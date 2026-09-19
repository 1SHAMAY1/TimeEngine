#include "PreRequisites.h"
#include "ECS/SystemScheduler.hpp"

TEArray<RegisteredSystem> SystemScheduler::s_Systems[static_cast<size_t>(ESystemPhase::Count)];

void SystemScheduler::Register(const TEString &name, ESystemPhase phase, SystemFunction func)
{
    size_t idx = static_cast<size_t>(phase);
    if (idx >= static_cast<size_t>(ESystemPhase::Count))
        return;

    for (const auto &sys : s_Systems[idx])
    {
        if (sys.Name == name)
            return;
    }

    s_Systems[idx].Add({name, phase, std::move(func)});
}

void SystemScheduler::Execute(ESystemPhase phase, EntityManager &manager, float dt)
{
    size_t idx = static_cast<size_t>(phase);
    if (idx >= static_cast<size_t>(ESystemPhase::Count))
        return;

    for (auto &sys : s_Systems[idx])
    {
        if (sys.Func)
        {
            sys.Func(manager, dt);
        }
    }
}

void SystemScheduler::Clear()
{
    for (size_t i = 0; i < static_cast<size_t>(ESystemPhase::Count); ++i)
    {
        s_Systems[i].Clear();
    }
}

const TEArray<RegisteredSystem> &SystemScheduler::GetSystems(ESystemPhase phase)
{
    size_t idx = static_cast<size_t>(phase);
    return s_Systems[idx];
}
