#pragma once
#include "Core/Collision/CollisionTypes.hpp"
#include <memory>
#include <vector>

namespace TE
{

struct RigidBody
{
    TEVector2 Position;
    TEVector2 Velocity;
    TEVector2 Force;
    float Mass = 1.0f;
    float InverseMass = 1.0f;
    float Restitution = 0.5f; // Bounciness
    bool IsStatic = false;
    bool IsSleeping = false;

    CollisionShape Shape;
    uint32_t m_VeloxEntityID = 0;

    void ApplyForce(const TEVector2 &force)
    {
        if (IsStatic)
            return;
        Force += force;
    }

    void Integrate(float dt)
    {
        if (IsStatic || InverseMass == 0.0f)
            return;

        // Symplectic Euler
        TEVector2 acceleration = Force * InverseMass;
        Velocity += acceleration * dt;
        Position += Velocity * dt;

        // Reset force
        Force = {0.0f, 0.0f};
    }
};

class PhysicsWorld
{
public:
    PhysicsWorld();
    ~PhysicsWorld();

    void AddBody(RigidBody *body);
    void RemoveBody(RigidBody *body);

    void Step(float dt);

    // Gravity Control
    void SetGravity(const TEVector2 &gravity);
    TEVector2 GetGravity() const { return m_Gravity; }

    // Joint System
    void AddDistanceJoint(uint32_t entityA, uint32_t entityB, const TEVector2 &anchorA, const TEVector2 &anchorB,
                          float targetDistance, float compliance);
    void AddRevoluteJoint(uint32_t entityA, uint32_t entityB, const TEVector2 &anchorA, const TEVector2 &anchorB,
                          float compliance, bool limitsEnabled = false, float lowerAngle = 0.0f,
                          float upperAngle = 0.0f, bool enableMotor = false, float motorSpeed = 0.0f,
                          float maxMotorTorque = 0.0f);
    void AddPrismaticJoint(uint32_t entityA, uint32_t entityB, const TEVector2 &anchorA, const TEVector2 &anchorB,
                           const TEVector2 &axisA, float compliance, bool limitsEnabled = false,
                           float minTranslation = 0.0f, float maxTranslation = 0.0f, bool enableMotor = false,
                           float motorSpeed = 0.0f, float maxMotorForce = 0.0f);
    void AddGearJoint(uint32_t entityA, uint32_t entityB, float gearRatio, float compliance);
    void AddPulleyJoint(uint32_t entityA, uint32_t entityB, const TEVector2 &groundA, const TEVector2 &groundB,
                        const TEVector2 &anchorA, const TEVector2 &anchorB, float ratio, float totalLength,
                        float compliance);

    // Sensor & Group Configuration
    void SetColliderSensor(uint32_t entityID, bool isSensor);
    void SetColliderGroupId(uint32_t entityID, int groupId);

    // Soft Body System
    uint32_t CreateSoftBodyBlob(const TEVector2 &center, float radius, int nodeCount, float compliance,
                                float jointCompliance, float nodeRadius);
    uint32_t CreateSoftBodyShapeMatched(const TEVector2 &center, float *verticesX, float *verticesY, int vertexCount,
                                        float stiffness, float nodeRadius);
    int GetSoftBodyNodeCount(uint32_t softBodyEntityID);
    uint32_t GetSoftBodyNode(uint32_t softBodyEntityID, int nodeIndex);

    // Raycasting
    bool Raycast(const TEVector2 &start, const TEVector2 &direction, float maxDistance, TEVector2 &hitPoint,
                 TEVector2 &hitNormal, float &fraction, uint32_t &hitEntityID);

private:
    std::vector<RigidBody *> m_Bodies;
    TEVector2 m_Gravity = {0.0f, -9.81f};
    void *m_VeloxWorld = nullptr;

    void ResolveCollisions();
};

} // namespace TE
