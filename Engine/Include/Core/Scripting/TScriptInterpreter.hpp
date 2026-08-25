#pragma once

#include "Core/Scripting/TScriptAST.hpp"
#include "Core/Scripting/TScriptValue.hpp"


// Forward declarations for engine types
class Scene;
class Entity;
struct InputActionBinding;

class TScriptInterpreter {
public:
    TScriptInterpreter() = default;

    void LoadProgram(const TScriptProgram& program);
    bool HasError() const { return !m_RuntimeError.empty(); }
    const TEString& GetRuntimeError() const { return m_RuntimeError; }

    // Bind entity context & reflection bridge
    void BindEntity(uint64_t entityID, Ref<Scene> scene);

    // Event dispatchers
    void DispatchReady();
    void DispatchUpdate(float dt);
    void DispatchCollision(uint64_t otherEntityID);
    void DispatchInput(const TEArray<InputActionBinding>& bindings);
    void DispatchTimer(const TEString& name);
    void DispatchDestroy();

    // Variable snapshotting for time reversal / rewind state
    TEMap<TEString, TScriptValue> SnapshotVariables() const;
    void RestoreVariables(const TEMap<TEString, TScriptValue>& snapshot);

    // Getters for editor inspection
    const TEMap<TEString, TScriptValue>& GetGlobals() const { return m_Globals; }
    void SetGlobal(const TEString& name, const TScriptValue& val) { m_Globals[name] = val; }

private:
    TScriptValue EvalExpr(const ExprNode& node);
    void ExecStmt(const StmtNode& node);
    void ExecBlock(const BlockNode& block);

    TScriptValue CallBuiltin(const TEString& name, const TEArray<TScriptValue>& args);
    TScriptValue GetProperty(const ExprNode& target, const TEString& field);
    void SetProperty(const ExprNode& target, const TEString& field, const TEString& op, const TScriptValue& val);

    TScriptProgram m_Program;
    TEMap<TEString, TScriptValue> m_Globals;
    TEMap<TEString, EventFuncNode*> m_EventMap;

    uint64_t m_BoundEntityID = 0;
    Scene* m_Scene = nullptr;
    TEString m_RuntimeError;
};

