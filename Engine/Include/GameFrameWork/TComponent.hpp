#pragma once
#include "Core/PreRequisites.h"
#include "Utility/MathUtils.hpp"
#include <algorithm>
#include <vector>
class TObject;

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
