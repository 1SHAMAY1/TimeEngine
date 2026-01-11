#include "Core/Physics/PhysicsWorld.hpp"
#include "Core/Log.h"
#include <algorithm>

namespace TE {

    void PhysicsWorld::AddBody(RigidBody* body) {
        m_Bodies.push_back(body);
    }

    void PhysicsWorld::RemoveBody(RigidBody* body) {
        // Simple linear removal for now
        auto it = std::find(m_Bodies.begin(), m_Bodies.end(), body);
        if (it != m_Bodies.end()) {
            m_Bodies.erase(it);
        }
    }

    void PhysicsWorld::Step(float dt) {
        // 1. Integrate Forces
        for (auto* body : m_Bodies) {
            if (!body->IsStatic) {
                body->ApplyForce(m_Gravity * body->Mass);
                body->Integrate(dt);
            }
        }

        // 2. Resolve Collisions (Naive O(N^2) for now)
        ResolveCollisions();
    }



    void PhysicsWorld::ResolveCollisions() {
        for (size_t i = 0; i < m_Bodies.size(); ++i) {
            for (size_t j = i + 1; j < m_Bodies.size(); ++j) {
                RigidBody* A = m_Bodies[i];
                RigidBody* B = m_Bodies[j];

                if (A->IsStatic && B->IsStatic) continue;

                if (A->Shape.type == CollisionType::AABB && B->Shape.type == CollisionType::AABB) {
                    
                    TEVector2 posA = A->Position;
                    TEVector2 posB = B->Position;
                    
                    TEVector2 minA = posA + A->Shape.aabb.min;
                    TEVector2 maxA = posA + A->Shape.aabb.max;
                    TEVector2 minB = posB + B->Shape.aabb.min;
                    TEVector2 maxB = posB + B->Shape.aabb.max;

                    // Check Overlap
                    if (minA.x < maxB.x && maxA.x > minB.x &&
                        minA.y < maxB.y && maxA.y > minB.y)
                    {
                        // Collision Detected
                        
                        // Calculate Penetration Depth
                        float d1 = maxB.x - minA.x;
                        float d2 = maxA.x - minB.x;
                        float d3 = maxB.y - minA.y;
                        float d4 = maxA.y - minB.y;
                        
                        // Find minimum penetration
                        float minOverlap = d1;
                        TEVector2 normal = { 1.0f, 0.0f }; // Normal points from A to B?
                        
                        if (d2 < minOverlap) { minOverlap = d2; normal = { -1.0f, 0.0f }; }
                        if (d3 < minOverlap) { minOverlap = d3; normal = { 0.0f, 1.0f }; }
                        if (d4 < minOverlap) { minOverlap = d4; normal = { 0.0f, -1.0f }; }
                        
                        // Positional Correction (Linear Projection)
                        const float percent = 0.8f; // Penetration percentage to correct
                        const float slop = 0.01f;   // Penetration allowance
                        TEVector2 correction = normal * (std::max(minOverlap - slop, 0.0f) * percent);
                        
                        float invMassA = A->InverseMass;
                        float invMassB = B->InverseMass;
                        float totalInvMass = invMassA + invMassB;
                        
                        if (totalInvMass == 0.0f) continue;
                        
                        if (!A->IsStatic) A->Position -= correction * (invMassA / totalInvMass);
                        if (!B->IsStatic) B->Position += correction * (invMassB / totalInvMass);
                        
                        // Impulse Resolution
                        TEVector2 rv = B->Velocity - A->Velocity;
                        float velAlongNormal = Dot(rv, normal);
                        
                        if (velAlongNormal > 0) continue; // Moving away
                        
                        float e = std::min(A->Restitution, B->Restitution);
                        float jVal = -(1 + e) * velAlongNormal;
                        jVal /= totalInvMass;
                        
                        TEVector2 impulse = normal * jVal;
                        
                        if (!A->IsStatic) A->Velocity -= impulse * invMassA;
                        if (!B->IsStatic) B->Velocity += impulse * invMassB;
                    }
                }
            }
        }
    }

}
