#include "PreRequisites.h"
#include "ECS/SystemScheduler.hpp"
#include "ECS/Query.hpp"
#include "StateTreeComponent.hpp"

void UpdateStateTreeSystem(EntityManager &manager, float dt)
{
    ComponentQuery<StateTreeComponent> stQuery(manager);
    stQuery.ForEach([dt](EntityID entityID, StateTreeComponent &st) {
        st.Tick(dt);
    });
}

struct StateTreeSystemRegistrar
{
    StateTreeSystemRegistrar()
    {
        SystemScheduler::Register("StateTreeSystem", ESystemPhase::Logic, UpdateStateTreeSystem);
    }
};

static StateTreeSystemRegistrar s_StateTreeSysReg;
