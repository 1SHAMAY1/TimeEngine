#pragma once
#include "GameFrameWork/TComponent.hpp"
#include "Core/Physics/PhysicsWorld.hpp"

#include "Core/Scene/ComponentRegistry.hpp"


    class RigidBodyComponent : public TComponent {
    public:
        GENERATED_BODY(RigidBodyComponent)

        // Internal Physics Body Data
        RigidBody Body;

        RigidBodyComponent() {
            // Default configuration
            Body.Mass = 1.0f;
            Body.InverseMass = 1.0f;
            Body.Position = { 0.0f, 0.0f };
        }

        void OnAttach() {
            // Register with PhysicsWorld if available globally or passed in
            // For now, let's assume we manage it externally or via a System
        }

        // Helpers
        void SetMass(float mass) {
            Body.Mass = mass;
            Body.InverseMass = (mass > 0.0f) ? 1.0f / mass : 0.0f;
        }

        void AddForce(const TEVector2& force) {
            Body.ApplyForce(force);
        }

        virtual TEString GetClassName() const override { return StaticClassName; }
    };

#ifdef TE_EDITOR
    T_REGISTER_COMPONENT(RigidBodyComponent, "RigidBody 2D Component")
    T_REGISTER_PRESET(RigidBodyComponent, "RigidBody 2D", "Physics & Collisions",
                      [](EntityID id, EntityManager *em) { em->AddComponent<RigidBodyComponent>(id); })
#endif

