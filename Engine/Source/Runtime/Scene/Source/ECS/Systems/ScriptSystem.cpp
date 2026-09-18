#include "PreRequisites.h"
#include "ECS/SystemScheduler.hpp"
#include "ECS/Query.hpp"
#include "ECS/ScriptComponent.hpp"

void UpdateScriptSystem(EntityManager &manager, float dt)
{
    ComponentQuery<ScriptComponent> scriptQuery(manager);
    scriptQuery.ForEach([dt](EntityID entityID, ScriptComponent &scriptComp) {
        scriptComp.DispatchScriptUpdate(dt);
    });
}

struct ScriptSystemRegistrar
{
    ScriptSystemRegistrar()
    {
        SystemScheduler::Register("ScriptSystem", ESystemPhase::Logic, UpdateScriptSystem);
    }
};

static ScriptSystemRegistrar s_ScriptSysReg;
