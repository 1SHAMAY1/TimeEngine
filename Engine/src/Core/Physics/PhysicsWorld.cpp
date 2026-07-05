#include "Core/Physics/PhysicsWorld.hpp"
#include "Core/Log.h"
#include <velox/VeloxAPI.h>
#include <algorithm>

namespace TE {

    PhysicsWorld::PhysicsWorld()
    {
        m_VeloxWorld = Velox_CreateWorld();
    }

    PhysicsWorld::~PhysicsWorld()
    {
        if (m_VeloxWorld)
        {
            Velox_DestroyWorld((VeloxWorld*)m_VeloxWorld);
            m_VeloxWorld = nullptr;
        }
    }

    void PhysicsWorld::AddBody(RigidBody* body) {
        m_Bodies.push_back(body);
        
        if (!m_VeloxWorld) return;

        // 1. Create Velox Entity
        uint32_t id = Velox_CreateEntity((VeloxWorld*)m_VeloxWorld);
        body->m_VeloxEntityID = id;

        // 2. Add Transform Component
        Velox_AddTransform((VeloxWorld*)m_VeloxWorld, id, body->Position.x, body->Position.y, 0.0f);

        // 3. Add RigidBody Component
        Velox_AddRigidBody((VeloxWorld*)m_VeloxWorld, id, body->Mass, body->IsStatic);

        // 4. Add Movement Component (stores velocity)
        Velox_AddMovement((VeloxWorld*)m_VeloxWorld, id);
        Velox_SetVelocity((VeloxWorld*)m_VeloxWorld, id, body->Velocity.x, body->Velocity.y);

        // 5. Add Physical Material Component
        Velox_AddPhysicalMaterial((VeloxWorld*)m_VeloxWorld, id, 0.5f, 0.3f, body->Restitution);

        // 6. Add Collider Component depending on shape
        if (body->Shape.type == CollisionType::AABB) {
            float width = body->Shape.aabb.max.x - body->Shape.aabb.min.x;
            float height = body->Shape.aabb.max.y - body->Shape.aabb.min.y;
            Velox_AddBoxCollider((VeloxWorld*)m_VeloxWorld, id, width, height);
        }
        else if (body->Shape.type == CollisionType::Circle) {
            Velox_AddCircleCollider((VeloxWorld*)m_VeloxWorld, id, body->Shape.circle.radius);
        }
        else {
            // Default to a fallback box collider if none defined
            Velox_AddBoxCollider((VeloxWorld*)m_VeloxWorld, id, 50.0f, 50.0f);
        }
    }

    void PhysicsWorld::RemoveBody(RigidBody* body) {
        auto it = std::find(m_Bodies.begin(), m_Bodies.end(), body);
        if (it != m_Bodies.end()) {
            m_Bodies.erase(it);
        }

        if (m_VeloxWorld && body->m_VeloxEntityID != 0) {
            Velox_DestroyEntity((VeloxWorld*)m_VeloxWorld, body->m_VeloxEntityID);
            body->m_VeloxEntityID = 0;
        }
    }

    void PhysicsWorld::Step(float dt) {
        if (!m_VeloxWorld) return;

        // 1. Sync any manual position/velocity updates from client code to Velox
        for (auto* body : m_Bodies) {
            if (body->m_VeloxEntityID != 0) {
                // Apply accumulated force if any
                if (body->Force.x != 0.0f || body->Force.y != 0.0f) {
                    if (!body->IsStatic && body->Mass > 0.0f) {
                        body->Velocity += (body->Force / body->Mass) * dt;
                    }
                    body->Force = {0.0f, 0.0f};
                }
                // Sync current velocity to Velox in case client code modified it
                Velox_SetVelocity((VeloxWorld*)m_VeloxWorld, body->m_VeloxEntityID, body->Velocity.x, body->Velocity.y);
            }
        }

        // 2. Step Velox Simulation
        Velox_Step((VeloxWorld*)m_VeloxWorld, dt);

        // 3. Query simulated results back to RigidBody structures
        for (auto* body : m_Bodies) {
            if (body->m_VeloxEntityID != 0) {
                float x = 0.0f, y = 0.0f, rot = 0.0f;
                Velox_GetPosition((VeloxWorld*)m_VeloxWorld, body->m_VeloxEntityID, &x, &y, &rot);
                body->Position = { x, y };
            }
        }
    }

    void PhysicsWorld::ResolveCollisions() {
        // Resolving is fully handled inside Velox_Step via XPBD solver now
    }

}
