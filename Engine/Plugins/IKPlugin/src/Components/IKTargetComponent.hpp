#pragma once

#include "Core/PreRequisites.h"
#include "Core/Scene/ComponentRegistry.hpp"
#include "GameFrameWork/TComponent.hpp"
#include "Utils/MathUtils.hpp"

class IKTarget2DComponent : public TComponent
{
public:
    GENERATED_BODY(IKTarget2DComponent)

    T_PROPERTY(bool, Enabled, "Enabled", true)
    T_PROPERTY(TEString, TargetTag, "Target Tag", "PlayerHandTarget")
    T_PROPERTY(TEVector2, TargetPositionOffset, "Offset", TEVector2(0.0f, 0.0f))

    virtual ~IKTarget2DComponent() override = default;

    virtual TEString GetClassName() const override { return StaticClassName; }

    void OnUpdate(float dt);
};

#ifdef TE_EDITOR
T_REGISTER_COMPONENT(IKTarget2DComponent, "IK Target 2D")
T_REGISTER_PROPERTY(IKTarget2DComponent, bool, Enabled, "Enabled")
T_REGISTER_PROPERTY(IKTarget2DComponent, TEString, TargetTag, "Target Tag")
T_REGISTER_PROPERTY(IKTarget2DComponent, TEVector2, TargetPositionOffset, "Offset")
#endif
