#pragma once
#include "Core/Collision/CollisionTypes.hpp"
#include <vector>
#include <memory>

namespace TE {

    struct RigidBody {
        TEVector2 Position;
        TEVector2 Velocity;
        TEVector2 Force;
        float Mass = 1.0f;
        float InverseMass = 1.0f;
        float Restitution = 0.5f; // Bounciness
        bool IsStatic = false;

        CollisionShape Shape;

        void ApplyForce(const TEVector2& force) {
            if (IsStatic) return;
            Force += force;
        }

        void Integrate(float dt) {
            if (IsStatic || InverseMass == 0.0f) return;

            // Symplectic Euler
            TEVector2 acceleration = Force * InverseMass;
            Velocity += acceleration * dt;
            Position += Velocity * dt;

            // Reset force
            Force = {0.0f, 0.0f};
        }
    };

    class PhysicsWorld {
    public:
        void AddBody(RigidBody* body);
        void RemoveBody(RigidBody* body);

        void Step(float dt);

    private:
        std::vector<RigidBody*> m_Bodies;
        TEVector2 m_Gravity = { 0.0f, -9.81f };

        void ResolveCollisions();
    };

}
