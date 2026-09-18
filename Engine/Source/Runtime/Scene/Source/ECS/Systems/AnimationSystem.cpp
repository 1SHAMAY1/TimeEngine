#include "PreRequisites.h"
#include "ECS/SystemScheduler.hpp"
#include "ECS/Query.hpp"
#include "AnimatedSpriteComponent.hpp"

void UpdateAnimationSystem(EntityManager &manager, float dt)
{
    ComponentQuery<AnimatedSpriteComponent> query(manager);
    query.ForEach([dt](EntityID entityID, AnimatedSpriteComponent &anim) {
        anim.Tick(dt);
    });
}

struct AnimationSystemRegistrar
{
    AnimationSystemRegistrar()
    {
        SystemScheduler::Register("AnimationSystem", ESystemPhase::Logic, UpdateAnimationSystem);
    }
};

static AnimationSystemRegistrar s_AnimSysReg;
