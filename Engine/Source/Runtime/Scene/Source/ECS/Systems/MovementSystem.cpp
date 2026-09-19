#include "PreRequisites.h"
#include "ECS/SystemScheduler.hpp"
#include "ECS/Query.hpp"
#include "CharacterBody2DComponent.hpp"
#include "MovementComponentBase.hpp"

void UpdateMovementSystem(EntityManager &manager, float dt)
{
    ComponentQuery<CharacterBody2DComponent> cbQuery(manager);
    cbQuery.ForEach([dt](EntityID entityID, CharacterBody2DComponent &cb) { cb.Tick(dt); });
}

struct MovementSystemRegistrar
{
    MovementSystemRegistrar()
    {
        SystemScheduler::Register("MovementSystem", ESystemPhase::Logic, UpdateMovementSystem);
    }
};

static MovementSystemRegistrar s_MoveSysReg;
