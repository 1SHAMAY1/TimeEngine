#pragma once
#include "ComponentRegistry.hpp"
#include "GameFrameWork/TComponent.hpp"

TE_CLASS()
class TE_API ControllableGameObject : public TComponent
{
public:
    GENERATED_BODY(ControllableGameObject)

    TEPROPERTY()
    bool bInputEnabled = true;

    virtual void EnableInput() { bInputEnabled = true; }
    virtual void DisableInput() { bInputEnabled = false; }
    bool IsInputEnabled() const { return bInputEnabled; }

    virtual TEString GetClassName() const override { return StaticClassName; }
};
