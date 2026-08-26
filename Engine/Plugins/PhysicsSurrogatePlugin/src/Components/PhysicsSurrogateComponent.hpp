#pragma once

#include "Core/Scene/ComponentRegistry.hpp"
#include "GameFrameWork/TComponent.hpp"
#include "Utils/MathUtils.hpp"

class PhysicsSurrogateComponent : public TComponent
{
public:
    GENERATED_BODY(PhysicsSurrogateComponent)

    T_PROPERTY(float, Damping, "Damping", 0.95f)
    T_PROPERTY(float, Elasticity, "Elasticity", 0.8f)

    virtual TEString GetClassName() const override { return StaticClassName; }

    void OnUpdate(float dt);
};

#ifdef TE_EDITOR
T_REGISTER_COMPONENT(PhysicsSurrogateComponent, "Physics Surrogate Component")
T_REGISTER_PROPERTY(PhysicsSurrogateComponent, float, Damping, "Damping")
T_REGISTER_PROPERTY(PhysicsSurrogateComponent, float, Elasticity, "Elasticity")
#endif
