#pragma once
#include "GameFrameWork/TComponent.hpp"
#include "Core/Physics/PhysicsWorld.hpp"

namespace TE {

    class RigidBodyComponent : public TComponent {
    public:
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

        static constexpr const char* StaticClassName = "RigidBodyComponent";
    };

}
