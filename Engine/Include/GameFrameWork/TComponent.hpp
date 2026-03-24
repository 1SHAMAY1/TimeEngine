#pragma once
#include "Core/PreRequisites.h"
#include "Utility/MathUtils.hpp"
#include "Utility/UIUtils.hpp"
#include <algorithm>
#include <glm/glm.hpp>
#include <memory>
#include <string>
#include <vector>

class TObject;

namespace TE
{
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
    std::vector<TComponent *> Children;

public:
    TEPROPERTY()
    std::string InstanceName;

    TE::TETransform Transform;
    virtual ~TComponent() = default;

    void SetOwner(TObject *newOwner) { Owner = newOwner; }
    TObject *GetOwner() const { return Owner; }

    void SetEntityManager(class EntityManager *mgr) { Manager = mgr; }
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

    virtual const char *GetClassName() const { return StaticClassName; }

    // ===== Universal Geometry Interface =====
    // Override these in subclasses to enable picking and shadow casting.

    /// Returns world-space outline vertices for this component (used by shadow casting).
    virtual std::vector<TE::TEVector2> GetWorldVertices(const glm::mat4 &worldModel) const { return {}; }

    /// Returns true if the world-space point is inside this component (used for picking/selection).
    virtual bool ContainsPoint(const glm::mat4 &worldModel, const TE::TEVector2 &point) const { return false; }

    /// Returns true if this component should block light (for shadow casting).
    virtual bool CastsOcclusionShadow() const { return false; }

    /// Renders the component specifically for the editor/scene view.
    virtual void OnRender(class TE::Renderer2D *renderer, const glm::mat4 &worldModel,
                          const std::shared_ptr<class TE::Material> &material) const
    {
    }

    // Hierarchy
    TComponent *GetParentComponent() const { return Parent; }
    const std::vector<TComponent *> &GetChildrenComponents() const { return Children; }

    void SetComponentParent(TComponent *newParent)
    {
        if (Parent == newParent)
            return;

        // Remove from old parent
        if (Parent)
        {
            auto it = std::find(Parent->Children.begin(), Parent->Children.end(), this);
            if (it != Parent->Children.end())
                Parent->Children.erase(it);
        }

        Parent = newParent;

        // Add to new parent
        if (Parent)
        {
            Parent->Children.push_back(this);
        }
    }

    static constexpr const char *StaticClassName = "TComponent";
};

} // namespace TE

#define TPROPERTY_FLOAT(var, name) ImGui::DragFloat(name, &var, 0.1f)
#define TPROPERTY_VEC2(var, name) TE::UIUtils::DrawVec2Control(name, *(glm::vec2 *)&var)
#define TPROPERTY_VEC3(var, name) TE::UIUtils::DrawVec3Control(name, var)
#define TPROPERTY_BOOL(var, name) ImGui::Checkbox(name, &var)
#define TPROPERTY_COLOR(var, name) ImGui::ColorEdit4(name, &var.GetValue().x)
