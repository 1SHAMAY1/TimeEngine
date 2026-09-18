#include "PreRequisites.h"
#include "ECS/SystemScheduler.hpp"
#include "ECS/Query.hpp"
#include "NavigationAgent2DComponent.hpp"

void UpdateNavigationSystem(EntityManager &manager, float dt)
{
    ComponentQuery<NavigationAgent2DComponent> navQuery(manager);
    navQuery.ForEach([dt](EntityID entityID, NavigationAgent2DComponent &nav) {
        nav.Tick(dt);
    });
}

struct NavigationSystemRegistrar
{
    NavigationSystemRegistrar()
    {
        SystemScheduler::Register("NavigationSystem", ESystemPhase::Logic, UpdateNavigationSystem);
    }
};

static NavigationSystemRegistrar s_NavSysReg;
