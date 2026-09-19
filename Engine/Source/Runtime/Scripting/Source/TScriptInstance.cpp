#include "PreRequisites.h"
#include "TScriptInstance.hpp"
#include "AssetManager.hpp"
#include "TScriptAsset.hpp"
#include "Scene.hpp"

void TScriptInstance::Init(uint64_t entityID, Ref<Scene> scene)
{
    if (ScriptHandle == 0)
        return;

    auto asset = AssetManager::GetAsset<TScriptAsset>(ScriptHandle);
    if (asset)
    {
        if (!asset->ASTValid)
        {
            asset->Recompile();
        }
        if (asset->ASTValid)
        {
            Interpreter.LoadProgram(asset->CachedAST);
            Interpreter.BindEntity(entityID, scene);
        }
    }
}

void TScriptInstance::DispatchReady()
{
    if (Enabled)
        Interpreter.DispatchReady();
}

void TScriptInstance::DispatchUpdate(float dt)
{
    if (Enabled)
        Interpreter.DispatchUpdate(dt);
}

void TScriptInstance::DispatchCollision(uint64_t otherEntityID)
{
    if (Enabled)
        Interpreter.DispatchCollision(otherEntityID);
}

void TScriptInstance::DispatchInput(const TEArray<InputActionBinding> &bindings)
{
    if (Enabled)
        Interpreter.DispatchInput(bindings);
}

void TScriptInstance::DispatchTimer(const TEString &name)
{
    if (Enabled)
        Interpreter.DispatchTimer(name);
}

void TScriptInstance::DispatchDestroy()
{
    if (Enabled)
        Interpreter.DispatchDestroy();
}
