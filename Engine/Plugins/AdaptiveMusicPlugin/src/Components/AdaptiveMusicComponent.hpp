#pragma once

#include "Core/Asset/Asset.hpp"
#include "Core/Scene/ComponentRegistry.hpp"
#include "GameFrameWork/TComponent.hpp"

class AdaptiveMusicComponent : public TComponent
{
public:
    GENERATED_BODY(AdaptiveMusicComponent)

    T_PROPERTY(AssetHandle, CalmStem, "Calm Stem", 0)
    T_PROPERTY(AssetHandle, TenseStem, "Tense Stem", 0)
    T_PROPERTY(AssetHandle, ActionStem, "Action Stem", 0)
    T_PROPERTY(float, TensionLevel, "Tension Level", 0.0f)

    virtual TEString GetClassName() const override { return StaticClassName; }

    void SetTension(float tension);
    void OnUpdate(float dt);
};

#ifdef TE_EDITOR
T_REGISTER_COMPONENT(AdaptiveMusicComponent, "Adaptive Music Component")
T_REGISTER_PROPERTY(AdaptiveMusicComponent, AssetHandle, CalmStem, "Calm Stem")
T_REGISTER_PROPERTY(AdaptiveMusicComponent, AssetHandle, TenseStem, "Tense Stem")
T_REGISTER_PROPERTY(AdaptiveMusicComponent, AssetHandle, ActionStem, "Action Stem")
T_REGISTER_PROPERTY(AdaptiveMusicComponent, float, TensionLevel, "Tension Level")
#endif
