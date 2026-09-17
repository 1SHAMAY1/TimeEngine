#pragma once

#include "PreRequisites.h"
#include "TScriptInterpreter.hpp"

using AssetHandle = uint64_t;

// Forward declaration
class Scene;

struct TE_API TScriptInstance
{
    AssetHandle ScriptHandle = 0;
    TScriptInterpreter Interpreter;
    bool Enabled = true;

    void Init(uint64_t entityID, Ref<Scene> scene);
    void DispatchReady();
    void DispatchUpdate(float dt);
    void DispatchCollision(uint64_t otherEntityID);
    void DispatchInput(const TEArray<InputActionBinding> &bindings);
    void DispatchTimer(const TEString &name);
    void DispatchDestroy();
};
