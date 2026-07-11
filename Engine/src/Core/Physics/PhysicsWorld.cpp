#include "Core/Physics/PhysicsWorld.hpp"
#include "Core/Log.h"
#include <algorithm>
#include <velox/VeloxAPI.h>

namespace TE
{

PhysicsWorld::PhysicsWorld()
{
    m_VeloxWorld = Velox_CreateWorld();
    if (m_VeloxWorld)
    {
        Velox_SetGravity((VeloxWorld *)m_VeloxWorld, m_Gravity.x, m_Gravity.y);
    }
}

PhysicsWorld::~PhysicsWorld()
{
    if (m_VeloxWorld)
    {
        Velox_DestroyWorld((VeloxWorld *)m_VeloxWorld);
        m_VeloxWorld = nullptr;
    }
}

void PhysicsWorld::AddBody(RigidBody *body)
{
    m_Bodies.push_back(body);

    if (!m_VeloxWorld)
        return;

    // 1. Create Velox Entity
    uint32_t id = Velox_CreateEntity((VeloxWorld *)m_VeloxWorld);
    body->m_VeloxEntityID = id;

    // 2. Add Transform Component
    Velox_AddTransform((VeloxWorld *)m_VeloxWorld, id, body->Position.x, body->Position.y, 0.0f);

    // 3. Add RigidBody Component
    Velox_AddRigidBody((VeloxWorld *)m_VeloxWorld, id, body->Mass, body->IsStatic);

    // 4. Add Movement Component (stores velocity)
    Velox_AddMovement((VeloxWorld *)m_VeloxWorld, id);
    Velox_SetVelocity((VeloxWorld *)m_VeloxWorld, id, body->Velocity.x, body->Velocity.y);

    // 5. Add Physical Material Component
    Velox_AddPhysicalMaterial((VeloxWorld *)m_VeloxWorld, id, 0.5f, 0.3f, body->Restitution);

    // 6. Add Collider Component depending on shape
    if (body->Shape.type == CollisionType::AABB)
    {
        float width = body->Shape.aabb.max.x - body->Shape.aabb.min.x;
        float height = body->Shape.aabb.max.y - body->Shape.aabb.min.y;
        Velox_AddBoxCollider((VeloxWorld *)m_VeloxWorld, id, width, height);
    }
    else if (body->Shape.type == CollisionType::Circle)
    {
        Velox_AddCircleCollider((VeloxWorld *)m_VeloxWorld, id, body->Shape.circle.radius);
    }
    else if (body->Shape.type == CollisionType::Triangle)
    {
        float verticesX[3] = { body->Shape.triangle.points[0].x, body->Shape.triangle.points[1].x, body->Shape.triangle.points[2].x };
        float verticesY[3] = { body->Shape.triangle.points[0].y, body->Shape.triangle.points[1].y, body->Shape.triangle.points[2].y };
        Velox_AddPolygonCollider((VeloxWorld *)m_VeloxWorld, id, verticesX, verticesY, 3);
    }
    else if (body->Shape.type == CollisionType::Polygon)
    {
        std::vector<float> verticesX;
        std::vector<float> verticesY;
        for (const auto& pt : body->Shape.polygon.points)
        {
            verticesX.push_back(pt.x);
            verticesY.push_back(pt.y);
        }
        if (!verticesX.empty())
        {
            Velox_AddPolygonCollider((VeloxWorld *)m_VeloxWorld, id, verticesX.data(), verticesY.data(), (int)verticesX.size());
        }
    }
    else
    {
        // Default to a fallback box collider if none defined
        Velox_AddBoxCollider((VeloxWorld *)m_VeloxWorld, id, 50.0f, 50.0f);
    }
}

void PhysicsWorld::RemoveBody(RigidBody *body)
{
    auto it = std::find(m_Bodies.begin(), m_Bodies.end(), body);
    if (it != m_Bodies.end())
    {
        m_Bodies.erase(it);
    }

    if (m_VeloxWorld && body->m_VeloxEntityID != 0)
    {
        Velox_DestroyEntity((VeloxWorld *)m_VeloxWorld, body->m_VeloxEntityID);
        body->m_VeloxEntityID = 0;
    }
}

void PhysicsWorld::Step(float dt)
{
    if (!m_VeloxWorld)
        return;

    // 1. Sync any manual position/velocity updates from client code to Velox
    for (auto *body : m_Bodies)
    {
        if (body->m_VeloxEntityID != 0)
        {
            // Apply accumulated force if any
            if (body->Force.x != 0.0f || body->Force.y != 0.0f)
            {
                if (!body->IsStatic && body->Mass > 0.0f)
                {
                    body->Velocity += (body->Force / body->Mass) * dt;
                }
                body->Force = {0.0f, 0.0f};
            }
            // Sync current velocity to Velox in case client code modified it
            Velox_SetVelocity((VeloxWorld *)m_VeloxWorld, body->m_VeloxEntityID, body->Velocity.x, body->Velocity.y);
        }
    }

    // 2. Step Velox Simulation
    Velox_Step((VeloxWorld *)m_VeloxWorld, dt);

    // 3. Query simulated results back to RigidBody structures
    for (auto *body : m_Bodies)
    {
        if (body->m_VeloxEntityID != 0)
        {
            float x = 0.0f, y = 0.0f, rot = 0.0f;
            Velox_GetPosition((VeloxWorld *)m_VeloxWorld, body->m_VeloxEntityID, &x, &y, &rot);
            body->Position = {x, y};
            body->IsSleeping = Velox_IsSleeping((VeloxWorld *)m_VeloxWorld, body->m_VeloxEntityID);
        }
    }
}

void PhysicsWorld::ResolveCollisions()
{
    // Resolving is fully handled inside Velox_Step via XPBD solver now
}

void PhysicsWorld::SetGravity(const TEVector2& gravity)
{
    m_Gravity = gravity;
    if (m_VeloxWorld)
    {
        Velox_SetGravity((VeloxWorld *)m_VeloxWorld, m_Gravity.x, m_Gravity.y);
    }
}

void PhysicsWorld::AddDistanceJoint(uint32_t entityA, uint32_t entityB, const TEVector2& anchorA, const TEVector2& anchorB, float targetDistance, float compliance)
{
    if (m_VeloxWorld)
    {
        Velox_AddDistanceJoint((VeloxWorld *)m_VeloxWorld, entityA, entityB, anchorA.x, anchorA.y, anchorB.x, anchorB.y, targetDistance, compliance);
    }
}

void PhysicsWorld::AddRevoluteJoint(uint32_t entityA, uint32_t entityB, const TEVector2& anchorA, const TEVector2& anchorB, float compliance, bool limitsEnabled, float lowerAngle, float upperAngle, bool enableMotor, float motorSpeed, float maxMotorTorque)
{
    if (m_VeloxWorld)
    {
        Velox_AddRevoluteJoint((VeloxWorld *)m_VeloxWorld, entityA, entityB, anchorA.x, anchorA.y, anchorB.x, anchorB.y, compliance, limitsEnabled, lowerAngle, upperAngle, enableMotor, motorSpeed, maxMotorTorque);
    }
}

void PhysicsWorld::AddPrismaticJoint(uint32_t entityA, uint32_t entityB, const TEVector2& anchorA, const TEVector2& anchorB, const TEVector2& axisA, float compliance, bool limitsEnabled, float minTranslation, float maxTranslation, bool enableMotor, float motorSpeed, float maxMotorForce)
{
    if (m_VeloxWorld)
    {
        Velox_AddPrismaticJoint((VeloxWorld *)m_VeloxWorld, entityA, entityB, anchorA.x, anchorA.y, anchorB.x, anchorB.y, axisA.x, axisA.y, compliance, limitsEnabled, minTranslation, maxTranslation, enableMotor, motorSpeed, maxMotorForce);
    }
}

void PhysicsWorld::AddGearJoint(uint32_t entityA, uint32_t entityB, float gearRatio, float compliance)
{
    if (m_VeloxWorld)
    {
        Velox_AddGearJoint((VeloxWorld *)m_VeloxWorld, entityA, entityB, gearRatio, compliance);
    }
}

void PhysicsWorld::AddPulleyJoint(uint32_t entityA, uint32_t entityB, const TEVector2& groundA, const TEVector2& groundB, const TEVector2& anchorA, const TEVector2& anchorB, float ratio, float totalLength, float compliance)
{
    if (m_VeloxWorld)
    {
        Velox_AddPulleyJoint((VeloxWorld *)m_VeloxWorld, entityA, entityB, groundA.x, groundA.y, groundB.x, groundB.y, anchorA.x, anchorA.y, anchorB.x, anchorB.y, ratio, totalLength, compliance);
    }
}

void PhysicsWorld::SetColliderSensor(uint32_t entityID, bool isSensor)
{
    if (m_VeloxWorld)
    {
        Velox_SetColliderSensor((VeloxWorld *)m_VeloxWorld, entityID, isSensor);
    }
}

void PhysicsWorld::SetColliderGroupId(uint32_t entityID, int groupId)
{
    if (m_VeloxWorld)
    {
        Velox_SetColliderGroupId((VeloxWorld *)m_VeloxWorld, entityID, groupId);
    }
}

uint32_t PhysicsWorld::CreateSoftBodyBlob(const TEVector2& center, float radius, int nodeCount, float compliance, float jointCompliance, float nodeRadius)
{
    if (m_VeloxWorld)
    {
        return Velox_CreateSoftBodyBlob((VeloxWorld *)m_VeloxWorld, center.x, center.y, radius, nodeCount, compliance, jointCompliance, nodeRadius);
    }
    return 0;
}

uint32_t PhysicsWorld::CreateSoftBodyShapeMatched(const TEVector2& center, float* verticesX, float* verticesY, int vertexCount, float stiffness, float nodeRadius)
{
    if (m_VeloxWorld)
    {
        return Velox_CreateSoftBodyShapeMatched((VeloxWorld *)m_VeloxWorld, center.x, center.y, verticesX, verticesY, vertexCount, stiffness, nodeRadius);
    }
    return 0;
}

int PhysicsWorld::GetSoftBodyNodeCount(uint32_t softBodyEntityID)
{
    if (m_VeloxWorld)
    {
        return Velox_GetSoftBodyNodeCount((VeloxWorld *)m_VeloxWorld, softBodyEntityID);
    }
    return 0;
}

uint32_t PhysicsWorld::GetSoftBodyNode(uint32_t softBodyEntityID, int nodeIndex)
{
    if (m_VeloxWorld)
    {
        return Velox_GetSoftBodyNode((VeloxWorld *)m_VeloxWorld, softBodyEntityID, nodeIndex);
    }
    return 0;
}

bool PhysicsWorld::Raycast(const TEVector2& start, const TEVector2& direction, float maxDistance, TEVector2& hitPoint, TEVector2& hitNormal, float& fraction, uint32_t& hitEntityID)
{
    if (m_VeloxWorld)
    {
        return Velox_Raycast((VeloxWorld *)m_VeloxWorld, start.x, start.y, direction.x, direction.y, maxDistance, &hitPoint.x, &hitPoint.y, &hitNormal.x, &hitNormal.y, &fraction, &hitEntityID);
    }
    return false;
}

} // namespace TE
