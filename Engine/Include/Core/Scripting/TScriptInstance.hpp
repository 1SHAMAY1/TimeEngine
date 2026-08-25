#pragma once

#include "Core/PreRequisites.h"
#include "Core/Asset/Asset.hpp"
#include "Core/Asset/AssetManager.hpp"
#include "Core/Scripting/TScriptAsset.hpp"
#include "Core/Scripting/TScriptInterpreter.hpp"

struct TScriptInstance {
    AssetHandle ScriptHandle = 0;
    TScriptInterpreter Interpreter;
    bool Enabled = true;

    void Init(uint64_t entityID, Ref<Scene> scene) {
        if (ScriptHandle == 0)
            return;

        auto asset = AssetManager::GetAsset<TScriptAsset>(ScriptHandle);
        if (asset) {
            if (!asset->ASTValid) {
                asset->Recompile();
            }
            if (asset->ASTValid) {
                Interpreter.LoadProgram(asset->CachedAST);
                Interpreter.BindEntity(entityID, scene);
            }
        }
    }

    void DispatchReady() {
        if (Enabled)
            Interpreter.DispatchReady();
    }

    void DispatchUpdate(float dt) {
        if (Enabled)
            Interpreter.DispatchUpdate(dt);
    }

    void DispatchCollision(uint64_t otherEntityID) {
        if (Enabled)
            Interpreter.DispatchCollision(otherEntityID);
    }

    void DispatchInput(const TEArray<InputActionBinding>& bindings) {
        if (Enabled)
            Interpreter.DispatchInput(bindings);
    }

    void DispatchTimer(const TEString& name) {
        if (Enabled)
            Interpreter.DispatchTimer(name);
    }

    void DispatchDestroy() {
        if (Enabled)
            Interpreter.DispatchDestroy();
    }
};
