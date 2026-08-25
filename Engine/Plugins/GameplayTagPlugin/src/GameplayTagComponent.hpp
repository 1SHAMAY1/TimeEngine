#pragma once

#include "GameFrameWork/TComponent.hpp"
#include "GameplayTagContainer.hpp"
#include "GameplayTagWidgets.hpp"


class GameplayTagComponent : public TComponent
{
public:
    GameplayTagContainer Container;

    GameplayTagComponent() = default;

    virtual TEString GetClassName() const override { return StaticClassName; }

    inline static const TEString StaticClassName = "GameplayTagComponent";

    // Query tag hierarchy
    bool HasTag(const TEString &tagString) const { return Container.HasTag(tagString); }

    bool has_tag(const TEString &tagString) const { return HasTag(tagString); }

    bool HasTagExact(const TEString &tagString) const { return Container.HasTagExact(tagString); }

    void AddTag(const TEString &tagString) { Container.AddTag(tagString); }

    void RemoveTag(const TEString &tagString) { Container.RemoveTag(tagString); }

    virtual void OnDrawInspector() override
    {
        DrawGameplayTagContainerWidget("Gameplay Tags", Container);
    }
};

