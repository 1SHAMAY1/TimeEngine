#pragma once

#include "Core/Scene/ComponentRegistry.hpp"
#include "Core/Scene/RigidBody2DComponent.hpp"

class TE_API StaticBody2DComponent : public TComponent
{
public:
    GENERATED_BODY(StaticBody2DComponent)

    T_PROPERTY(bool, Enabled, "Enabled", true)

    virtual void OnInitialize() override
    {
        TComponent::OnInitialize();
        auto *rb = GetOwnerEntity().GetComponent<RigidBody2DComponent>();
        if (!rb)
        {
            rb = GetOwnerEntity().AddComponent<RigidBody2DComponent>();
        }
        if (rb)
        {
            rb->SetRigidBodyType(ERigidBodyType2D::Static);
            rb->Mass = 0.0f;
        }
    }

    virtual TEString GetClassName() const override { return StaticClassName; }
};

#ifdef TE_EDITOR
T_REGISTER_COMPONENT(StaticBody2DComponent, "StaticBody 2D Component")
T_REGISTER_PROPERTY(StaticBody2DComponent, bool, Enabled, "Enabled")
T_REGISTER_PRESET(StaticBody2DComponent, "Static Body 2D", "Physics & Collisions",
                  [](EntityID id, EntityManager *em)
                  {
                      em->AddComponent<StaticBody2DComponent>(id);
                      auto *rb = em->AddComponent<RigidBody2DComponent>(id);
                      if (rb)
                      {
                          rb->SetRigidBodyType(ERigidBodyType2D::Static);
                          rb->Mass = 0.0f;
                      }
                  })
#endif
