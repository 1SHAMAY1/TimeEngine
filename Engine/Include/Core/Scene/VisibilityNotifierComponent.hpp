#pragma once

#include "GameFrameWork/TComponent.hpp"
#include "Core/Scene/ComponentRegistry.hpp"
#include "Utils/MathUtils.hpp"

TE_CLASS()
class TE_API VisibilityNotifierComponent : public TComponent
{
public:
    TEPROPERTY()
    TEVector2 BoundsSize = {32.0f, 32.0f};

    VisibilityNotifierComponent() = default;
    virtual ~VisibilityNotifierComponent() override = default;

    bool IsOnScreen() const { return m_bIsOnScreen; }

    virtual void Tick(float deltaTime) override;

    inline static const TEString StaticClassName = "VisibilityNotifierComponent";
    virtual TEString GetClassName() const override { return StaticClassName; }

private:
    bool m_bIsOnScreen = false;
};

#ifdef TE_EDITOR
T_REGISTER_COMPONENT(VisibilityNotifierComponent, "Visibility Notifier Component")
T_REGISTER_PROPERTY(VisibilityNotifierComponent, TEVector2, BoundsSize, "Bounds Size")
T_REGISTER_PRESET(VisibilityNotifierPreset, "Visibility Notifier", "Utility", [](EntityID id, EntityManager *em) {
    em->AddComponent<VisibilityNotifierComponent>(id);
})
#endif
