#pragma once
#include "Core/PreRequisites.h"
#include "GameFrameWork/GameplayUtils.hpp"
#include "Utils/MathUtils.hpp"
#include "Utils/TimeGUI.hpp"
#include <algorithm>

#include "Core/Scripting/TScriptMacros.hpp"
#include "Core/Scripting/TScriptInstance.hpp"

class TObject;

class Renderer2D;
class Material;

TE_CLASS()
class TE_API TComponent
{
protected:
    TEPROPERTY()
    TObject *Owner = nullptr;

    class EntityManager *Manager = nullptr;

    TEPROPERTY()
    bool bMarkedPendingDestroy = false;

    TEPROPERTY()
    bool bInitialized = false;

    TComponent *Parent = nullptr;
    TEArray<TComponent *> Children;

    TEArray<TScriptInstance> m_Scripts;

public:
    TEPROPERTY()
    TEString InstanceName;

    TETransform Transform;
    virtual ~TComponent() = default;

    void SetOwner(TObject *newOwner) { Owner = newOwner; }
    TObject *GetOwner() const { return Owner; }

    void SetEntityManager(EntityManager *mgr) { Manager = mgr; }
    class EntityManager *GetEntityManager() const { return Manager; }

    // Convenience method to get the owner as an Entity
    class Entity GetOwnerEntity() const;

    virtual void OnInitialize() { bInitialized = true; }
    virtual void OnAttach() {}
    virtual void OnDetach() {}
    virtual void Tick(float deltaTime) {}

    void MarkPendingDestroy() { bMarkedPendingDestroy = true; }
    bool IsMarkedPendingDestroy() const { return bMarkedPendingDestroy; }
    bool IsInitialized() const { return bInitialized; }

    virtual void OnDrawInspector() {}

    virtual TEString GetClassName() const { return StaticClassName; }

    // ===== Script Management =====
    void AddScript(AssetHandle scriptHandle)
    {
        if (scriptHandle == 0)
            return;
        for (const auto &slot : m_Scripts)
        {
            if (slot.ScriptHandle == scriptHandle)
                return;
        }
        TScriptInstance instance;
        instance.ScriptHandle = scriptHandle;
        instance.Enabled = true;
        m_Scripts.Add(instance);
    }

    void RemoveScript(AssetHandle scriptHandle)
    {
        for (int i = 0; i < m_Scripts.size(); i++)
        {
            if (m_Scripts[i].ScriptHandle == scriptHandle)
            {
                m_Scripts.RemoveAt(i);
                break;
            }
        }
    }

    void RemoveScriptAt(size_t index)
    {
        if (index < m_Scripts.size())
            m_Scripts.RemoveAt(index);
    }

    TEArray<TScriptInstance> &GetScripts() { return m_Scripts; }
    const TEArray<TScriptInstance> &GetScripts() const { return m_Scripts; }

    void InitScripts(uint64_t entityID, Ref<Scene> scene)
    {
        for (auto &script : m_Scripts)
        {
            script.Init(entityID, scene);
        }
    }

    void DispatchScriptReady()
    {
        for (auto &script : m_Scripts)
            script.DispatchReady();
    }

    void DispatchScriptUpdate(float dt)
    {
        for (auto &script : m_Scripts)
            script.DispatchUpdate(dt);
    }

    void DispatchScriptCollision(uint64_t otherEntityID)
    {
        for (auto &script : m_Scripts)
            script.DispatchCollision(otherEntityID);
    }

    void DispatchScriptInput(const TEArray<InputActionBinding> &bindings)
    {
        for (auto &script : m_Scripts)
            script.DispatchInput(bindings);
    }

    void DispatchScriptTimer(const TEString &name)
    {
        for (auto &script : m_Scripts)
            script.DispatchTimer(name);
    }

    void DispatchScriptDestroy()
    {
        for (auto &script : m_Scripts)
            script.DispatchDestroy();
    }

    // ===== T_EVENT_VISIBLE Query =====
    virtual uint32_t GetEventVisibilityMask() const { return 0; }
    bool HasEventVisibility(TScriptEventType type) const
    {
        return (GetEventVisibilityMask() & static_cast<uint32_t>(type)) != 0;
    }

    // ===== Universal Geometry Interface =====
    // Override these in subclasses to enable picking and shadow casting.

    /// Returns world-space outline vertices for this component (used by shadow casting).
    virtual TEArray<TEVector2> GetWorldVertices(const TEMatrix4 &worldModel) const { return {}; }

    /// Returns true if the world-space point is inside this component (used for picking/selection).
    virtual bool ContainsPoint(const TEMatrix4 &worldModel, const TEVector2 &point) const
    {
        TEArray<TEVector2> vertices = GetWorldVertices(worldModel);
        if (vertices.Num() < 3)
            return false;

        bool inside = false;
        size_t n = vertices.Num();
        for (size_t i = 0, j = n - 1; i < n; j = i++)
        {
            float xi = vertices[i].x, yi = vertices[i].y;
            float xj = vertices[j].x, yj = vertices[j].y;

            bool intersect = ((yi > point.y) != (yj > point.y)) &&
                             (point.x < (xj - xi) * (point.y - yi) / (yj - yi + 0.0000001f) + xi);
            if (intersect)
                inside = !inside;
        }
        return inside;
    }

    /// Returns true if this component should block light (for shadow casting).
    virtual bool CastsOcclusionShadow() const { return false; }

    /// Renders the component specifically for the editor/scene view.
    virtual void OnRender(class Renderer2D *renderer, const TEMatrix4 &worldModel,
                          const TERef<class Material> &material) const
    {
    }

    // Hierarchy
    TComponent *GetParentComponent() const { return Parent; }
    const TEArray<TComponent *> &GetChildrenComponents() const { return Children; }

    void SetComponentParent(TComponent *newParent)
    {
        if (Parent == newParent)
            return;

        // Remove from old parent
        if (Parent)
        {
            Parent->Children.Remove(this);
        }

        Parent = newParent;

        // Add to new parent
        if (Parent)
        {
            Parent->Children.Add(this);
        }
    }

    inline static const TEString StaticClassName = "TComponent";
};


#define TPROPERTY_FLOAT(var, name) TimeGUI::DragFloat(name, &var)
#define TPROPERTY_VEC2(var, name) UIUtils::DrawVec2Control(name, var)
#define TPROPERTY_VEC3(var, name) UIUtils::DrawVec3Control(name, var)
#define TPROPERTY_BOOL(var, name) TimeGUI::Checkbox(name, &var)
#define TPROPERTY_COLOR(var, name) TimeGUI::DrawColorControl(name, var)

