#pragma once
#include "PreRequisites.h"
#include <functional>

class EntityManager;

enum class ESystemPhase : uint8_t
{
    PrePhysics = 0,
    Physics = 1,
    PostPhysics = 2,
    Logic = 3,
    Render = 4,
    Count = 5
};

using SystemFunction = std::function<void(EntityManager &, float)>;

struct RegisteredSystem
{
    TEString Name;
    ESystemPhase Phase = ESystemPhase::Logic;
    SystemFunction Func;
};

class TE_API SystemScheduler
{
public:
    static void Register(const TEString &name, ESystemPhase phase, SystemFunction func);
    static void Execute(ESystemPhase phase, EntityManager &manager, float dt);
    static void Clear();
    static const TEArray<RegisteredSystem> &GetSystems(ESystemPhase phase);

private:
    static TEArray<RegisteredSystem> s_Systems[static_cast<size_t>(ESystemPhase::Count)];
};
