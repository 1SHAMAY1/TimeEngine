#pragma once
#include "Core/PreRequisites.h"
#include "Utility/MathUtils.hpp"
#include <algorithm>
#include <memory>
#include <vector>
class TObject;

namespace TE
{
class Renderer2D;
class Material;
}

TE_CLASS()
class TE_API TComponent
{
protected:
    TEPROPERTY()
    TObject *Owner = nullptr;

    TEPROPERTY()
    bool bMarkedPendingDestroy = false;

    TEPROPERTY()
    bool bInitialized = false;

    TComponent *Parent = nullptr;
    std::vector<TComponent *> Children;

public:
    TE::TETransform Transform;
    virtual ~TComponent() = default;

    void SetOwner(TObject *newOwner) { Owner = newOwner; }
    TObject *GetOwner() const { return Owner; }

    void OnInitialize() { bInitialized = true; }
    void OnAttach() {}
    void OnDetach() {}
    void Tick(float deltaTime) {}

    void MarkPendingDestroy() { bMarkedPendingDestroy = true; }
    bool IsMarkedPendingDestroy() const { return bMarkedPendingDestroy; }
    bool IsInitialized() const { return bInitialized; }

    virtual const char *GetClassName() const { return StaticClassName; }

    // ===== Universal Geometry Interface =====
    // Override these in subclasses to enable picking and shadow casting.

    /// Returns world-space outline vertices for this component (used by shadow casting).
    /// Default: empty (no shadow). Override in shape components.
    virtual std::vector<TE::TEVector2> GetWorldVertices(const glm::mat4 &worldModel) const { return {}; }

    /// Returns true if the world-space point is inside this component (used for picking/selection).
    /// Default: false. Override in shape/light components.
    virtual bool ContainsPoint(const glm::mat4 &worldModel, const TE::TEVector2 &point) const { return false; }

    /// Returns true if this component should block light (for shadow casting).
    /// Default: false. Override in shape components with collision.
    virtual bool CastsOcclusionShadow() const { return false; }

    /// Renders the component specifically for the editor/scene view.
    /// Default: does nothing. Override in visible components.
    /// Renders the component specifically for the editor/scene view.
    /// Default: does nothing. Override in visible components.
    virtual void OnRender(class TE::Renderer2D *renderer, const glm::mat4 &worldModel, const std::shared_ptr<class TE::Material> &material) const {}

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
