#pragma once
#include "PreRequisites.h"
#include "TComponent.hpp"
#include "TScriptInstance.hpp"
#include "ComponentRegistry.hpp"

class Scene;

struct TE_API ScriptComponent : public TComponent
{
    GENERATED_BODY(ScriptComponent)

    TEArray<TScriptInstance> Scripts;

    void AddScript(AssetHandle scriptHandle)
    {
        if (scriptHandle == 0)
            return;
        for (const auto &slot : Scripts)
        {
            if (slot.ScriptHandle == scriptHandle)
                return;
        }
        TScriptInstance instance;
        instance.ScriptHandle = scriptHandle;
        instance.Enabled = true;
        Scripts.Add(instance);
    }

    void RemoveScript(AssetHandle scriptHandle)
    {
        for (size_t i = 0; i < Scripts.Num(); ++i)
        {
            if (Scripts[i].ScriptHandle == scriptHandle)
            {
                Scripts.RemoveAt(i);
                break;
            }
        }
    }

    void RemoveScriptAt(size_t index)
    {
        if (index < Scripts.Num())
            Scripts.RemoveAt(index);
    }

    void InitScripts(uint64_t entityID, const TERef<Scene> &scene)
    {
        for (auto &script : Scripts)
        {
            script.Init(entityID, scene);
        }
    }

    void DispatchScriptReady()
    {
        for (auto &script : Scripts)
            script.DispatchReady();
    }

    void DispatchScriptUpdate(float dt)
    {
        for (auto &script : Scripts)
            script.DispatchUpdate(dt);
    }

    void DispatchScriptCollision(uint64_t otherEntityID)
    {
        for (auto &script : Scripts)
            script.DispatchCollision(otherEntityID);
    }

    void DispatchScriptDestroy()
    {
        for (auto &script : Scripts)
            script.DispatchDestroy();
    }

    virtual TEString GetClassName() const override { return StaticClassName; }
};
