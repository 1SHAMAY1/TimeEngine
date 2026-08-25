#pragma once
#include "ControllableGameObject.hpp"
#include "Core/Scene/ComponentRegistry.hpp"


enum class ECameraSelectionMode
{
    DedicatedPlayerCamera = 0, // Smooth Follow & Shake & Deflect
    SceneOrthographicCamera = 1,
    PerspectiveCamera = 2,
    None = 3
};

TE_CLASS()
class TE_API PlayerBase : public ControllableGameObject
{
public:
    GENERATED_BODY(PlayerBase)

    TEPROPERTY()
    ECameraSelectionMode SelectedCameraMode = ECameraSelectionMode::DedicatedPlayerCamera;

    virtual void Update(float dt) = 0;

    virtual TEString GetClassName() const override { return StaticClassName; }
};

#ifdef TE_EDITOR
T_REGISTER_ENUM_PROPERTY(PlayerBase, ECameraSelectionMode, SelectedCameraMode, "Camera Selection Mode")
#endif

