#include "PreRequisites.h"
#include "PhysicsWorld.hpp"
#include "Log.h"
#include "Layers/ProfilingLayer.hpp"
#include <algorithm>
#include <chrono>
#include <velox/VeloxAPI.h>

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
    m_Bodies.Add(body);

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
    Velox_AddPhysicalMaterial((VeloxWorld *)m_VeloxWorld, id, body->StaticFriction, body->DynamicFriction,
                              body->Restitution);

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
        float verticesX[3] = {body->Shape.triangle.points[0].x, body->Shape.triangle.points[1].x,
                              body->Shape.triangle.points[2].x};
        float verticesY[3] = {body->Shape.triangle.points[0].y, body->Shape.triangle.points[1].y,
                              body->Shape.triangle.points[2].y};
        Velox_AddPolygonCollider((VeloxWorld *)m_VeloxWorld, id, verticesX, verticesY, 3);
    }
    else if (body->Shape.type == CollisionType::Polygon)
    {
        TEArray<float> verticesX;
        TEArray<float> verticesY;
        for (const auto &pt : body->Shape.polygon.points)
        {
            verticesX.Add(pt.x);
            verticesY.Add(pt.y);
        }
        if (!verticesX.IsEmpty())
        {
            Velox_AddPolygonCollider((VeloxWorld *)m_VeloxWorld, id, verticesX.Data(), verticesY.Data(),
                                     (int)verticesX.Size());
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
    m_Bodies.Remove(body);

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

    StackProfileScope scope("PhysicsWorld::Step", sizeof(PhysicsWorld) + sizeof(dt));

    auto startTime = std::chrono::high_resolution_clock::now();

    // 1. Sync manual forces/velocities for dynamic bodies
    for (auto *body : m_Bodies)
    {
        if (body->m_VeloxEntityID != 0 && !body->IsStatic && (body->Force.x != 0.0f || body->Force.y != 0.0f))
        {
            if (body->Mass > 0.0f)
            {
                body->Velocity += (body->Force / body->Mass) * dt;
                Velox_SetVelocity((VeloxWorld *)m_VeloxWorld, body->m_VeloxEntityID, body->Velocity.x,
                                  body->Velocity.y);
            }
            body->Force = {0.0f, 0.0f};
        }
    }

    // 2. Step Velox Simulation
    Velox_Step((VeloxWorld *)m_VeloxWorld, dt);

    // 3. Query simulated results back to RigidBody structures
    for (auto *body : m_Bodies)
    {
        if (body->m_VeloxEntityID != 0 && !body->IsStatic)
        {
            bool sleeping = Velox_IsSleeping((VeloxWorld *)m_VeloxWorld, body->m_VeloxEntityID);
            if (!sleeping || !body->IsSleeping)
            {
                float x = 0.0f, y = 0.0f, rot = 0.0f;
                Velox_GetPosition((VeloxWorld *)m_VeloxWorld, body->m_VeloxEntityID, &x, &y, &rot);
                body->Position = {x, y};
            }
            body->IsSleeping = sleeping;
        }
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    float durationMs = std::chrono::duration<float, std::milli>(endTime - startTime).count();

    if (auto *profiler = ProfilingLayer::GetInstance())
    {
        profiler->RecordPhysicsTime(durationMs);
    }
}

void PhysicsWorld::ResolveCollisions()
{
    // Resolving is fully handled inside Velox_Step via XPBD solver now
}

void PhysicsWorld::SetGravity(const TEVector2 &gravity)
{
    m_Gravity = gravity;
    if (m_VeloxWorld)
    {
        Velox_SetGravity((VeloxWorld *)m_VeloxWorld, m_Gravity.x, m_Gravity.y);
    }
}

void PhysicsWorld::AddDistanceJoint(uint32_t entityA, uint32_t entityB, const TEVector2 &anchorA,
                                    const TEVector2 &anchorB, float targetDistance, float compliance)
{
    if (m_VeloxWorld)
    {
        Velox_AddDistanceJoint((VeloxWorld *)m_VeloxWorld, entityA, entityB, anchorA.x, anchorA.y, anchorB.x, anchorB.y,
                               targetDistance, compliance);
    }
}

void PhysicsWorld::AddRevoluteJoint(uint32_t entityA, uint32_t entityB, const TEVector2 &anchorA,
                                    const TEVector2 &anchorB, float compliance, bool limitsEnabled, float lowerAngle,
                                    float upperAngle, bool enableMotor, float motorSpeed, float maxMotorTorque)
{
    if (m_VeloxWorld)
    {
        Velox_AddRevoluteJoint((VeloxWorld *)m_VeloxWorld, entityA, entityB, anchorA.x, anchorA.y, anchorB.x, anchorB.y,
                               compliance, limitsEnabled, lowerAngle, upperAngle, enableMotor, motorSpeed,
                               maxMotorTorque);
    }
}

void PhysicsWorld::AddPrismaticJoint(uint32_t entityA, uint32_t entityB, const TEVector2 &anchorA,
                                     const TEVector2 &anchorB, const TEVector2 &axisA, float compliance,
                                     bool limitsEnabled, float minTranslation, float maxTranslation, bool enableMotor,
                                     float motorSpeed, float maxMotorForce)
{
    if (m_VeloxWorld)
    {
        Velox_AddPrismaticJoint((VeloxWorld *)m_VeloxWorld, entityA, entityB, anchorA.x, anchorA.y, anchorB.x,
                                anchorB.y, axisA.x, axisA.y, compliance, limitsEnabled, minTranslation, maxTranslation,
                                enableMotor, motorSpeed, maxMotorForce);
    }
}

void PhysicsWorld::AddGearJoint(uint32_t entityA, uint32_t entityB, float gearRatio, float compliance)
{
    if (m_VeloxWorld)
    {
        Velox_AddGearJoint((VeloxWorld *)m_VeloxWorld, entityA, entityB, gearRatio, compliance);
    }
}

void PhysicsWorld::AddPulleyJoint(uint32_t entityA, uint32_t entityB, const TEVector2 &groundA,
                                  const TEVector2 &groundB, const TEVector2 &anchorA, const TEVector2 &anchorB,
                                  float ratio, float totalLength, float compliance)
{
    if (m_VeloxWorld)
    {
        Velox_AddPulleyJoint((VeloxWorld *)m_VeloxWorld, entityA, entityB, groundA.x, groundA.y, groundB.x, groundB.y,
                             anchorA.x, anchorA.y, anchorB.x, anchorB.y, ratio, totalLength, compliance);
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

void PhysicsWorld::AddRotation(uint32_t entityID, float speed, int direction, int mode)
{
    if (m_VeloxWorld)
    {
        Velox_AddRotation((VeloxWorld *)m_VeloxWorld, entityID, speed, direction, mode);
    }
}

void PhysicsWorld::AddOscillation(uint32_t entityID, const TEVector2 &axis, float amplitude, float frequency)
{
    if (m_VeloxWorld)
    {
        Velox_AddOscillation((VeloxWorld *)m_VeloxWorld, entityID, axis.x, axis.y, amplitude, frequency);
    }
}

void PhysicsWorld::AddProjectile(uint32_t entityID, bool faceVelocity, float speed, float maxSpeed, float bounceFactor)
{
    if (m_VeloxWorld)
    {
        Velox_AddProjectile((VeloxWorld *)m_VeloxWorld, entityID, faceVelocity, speed, maxSpeed, bounceFactor);
    }
}

void PhysicsWorld::AddPhysicalMaterial(uint32_t entityID, float staticFriction, float dynamicFriction,
                                       float restitution)
{
    if (m_VeloxWorld)
    {
        Velox_AddPhysicalMaterial((VeloxWorld *)m_VeloxWorld, entityID, staticFriction, dynamicFriction, restitution);
    }
}

void PhysicsWorld::AddForceField(uint32_t entityID, int type, float strength, float radius)
{
    if (m_VeloxWorld)
    {
        Velox_AddForceField((VeloxWorld *)m_VeloxWorld, entityID, type, strength, radius);
    }
}

void PhysicsWorld::WakeBody(uint32_t entityID)
{
    if (m_VeloxWorld)
    {
        Velox_WakeBody((VeloxWorld *)m_VeloxWorld, entityID);
    }
}

void PhysicsWorld::WakeTouching(uint32_t entityID)
{
    if (m_VeloxWorld)
    {
        Velox_WakeTouching((VeloxWorld *)m_VeloxWorld, entityID);
    }
}

bool PhysicsWorld::IsSleeping(uint32_t entityID)
{
    if (m_VeloxWorld)
    {
        return Velox_IsSleeping((VeloxWorld *)m_VeloxWorld, entityID);
    }
    return false;
}

uint32_t PhysicsWorld::CreateSoftBodyBlob(const TEVector2 &center, float radius, int nodeCount, float compliance,
                                          float jointCompliance, float nodeRadius)
{
    if (m_VeloxWorld)
    {
        return Velox_CreateSoftBodyBlob((VeloxWorld *)m_VeloxWorld, center.x, center.y, radius, nodeCount, compliance,
                                        jointCompliance, nodeRadius);
    }
    return 0;
}

uint32_t PhysicsWorld::CreateSoftBodyShapeMatched(const TEVector2 &center, float *verticesX, float *verticesY,
                                                  int vertexCount, float stiffness, float nodeRadius)
{
    if (m_VeloxWorld)
    {
        return Velox_CreateSoftBodyShapeMatched((VeloxWorld *)m_VeloxWorld, center.x, center.y, verticesX, verticesY,
                                                vertexCount, stiffness, nodeRadius);
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

bool PhysicsWorld::Raycast(const TEVector2 &start, const TEVector2 &direction, float maxDistance, TEVector2 &hitPoint,
                           TEVector2 &hitNormal, float &fraction, uint32_t &hitEntityID)
{
    if (m_VeloxWorld)
    {
        return Velox_Raycast((VeloxWorld *)m_VeloxWorld, start.x, start.y, direction.x, direction.y, maxDistance,
                             &hitPoint.x, &hitPoint.y, &hitNormal.x, &hitNormal.y, &fraction, &hitEntityID);
    }
    TEVector2 end = start + direction * maxDistance;
    TESpatialHitResult hit = SweepLine(start, end);
    if (hit.Hit)
    {
        hitPoint = hit.Point;
        hitNormal = hit.Normal;
        fraction = hit.Fraction;
        hitEntityID = hit.EntityID;
        return true;
    }
    return false;
}

static inline float Cross2D(const TEVector2 &a, const TEVector2 &b) { return a.x * b.y - a.y * b.x; }

static inline float Dot2D(const TEVector2 &a, const TEVector2 &b) { return a.x * b.x + a.y * b.y; }

static inline TEVector2 RotateVec2(const TEVector2 &v, float rad)
{
    float c = std::cos(rad);
    float s = std::sin(rad);
    return {v.x * c - v.y * s, v.x * s + v.y * c};
}

static bool RaycastSegment(const TEVector2 &p, const TEVector2 &d, const TEVector2 &a, const TEVector2 &b, float &outT,
                           TEVector2 &outNormal)
{
    TEVector2 edge = {b.x - a.x, b.y - a.y};
    float denom = Cross2D(d, edge);
    if (std::abs(denom) < 1e-6f)
        return false;

    TEVector2 ap = {a.x - p.x, a.y - p.y};
    float t = Cross2D(ap, edge) / denom;
    float u = Cross2D(ap, d) / denom;

    if (t >= 0.0f && t <= 1.0f && u >= 0.0f && u <= 1.0f)
    {
        outT = t;
        TEVector2 normal = {-edge.y, edge.x};
        float len = std::sqrt(normal.x * normal.x + normal.y * normal.y);
        if (len > 1e-6f)
            normal = normal / len;
        if (Dot2D(normal, d) > 0.0f)
            normal = -normal;
        outNormal = normal;
        return true;
    }
    return false;
}

static bool RaycastCircle(const TEVector2 &p, const TEVector2 &d, const TEVector2 &center, float radius, float &outT,
                          TEVector2 &outNormal, bool &outPenetrating)
{
    TEVector2 f = {p.x - center.x, p.y - center.y};
    float a = Dot2D(d, d);
    float b = 2.0f * Dot2D(f, d);
    float c = Dot2D(f, f) - radius * radius;

    if (c <= 0.0f)
    {
        outT = 0.0f;
        outPenetrating = true;
        float flen = std::sqrt(f.x * f.x + f.y * f.y);
        outNormal = (flen > 1e-6f) ? (f / flen) : TEVector2{0.0f, 1.0f};
        return true;
    }

    if (a < 1e-6f)
        return false;

    float disc = b * b - 4.0f * a * c;
    if (disc < 0.0f)
        return false;

    float sqrtDisc = std::sqrt(disc);
    float t = (-b - sqrtDisc) / (2.0f * a);
    if (t >= 0.0f && t <= 1.0f)
    {
        outT = t;
        outPenetrating = false;
        TEVector2 hitP = p + d * t;
        TEVector2 diff = hitP - center;
        float len = std::sqrt(diff.x * diff.x + diff.y * diff.y);
        outNormal = (len > 1e-6f) ? (diff / len) : TEVector2{0.0f, 1.0f};
        return true;
    }
    return false;
}

static void GetBodyPolygonPoints(const RigidBody *body, TEArray<TEVector2> &outPoints)
{
    outPoints.Clear();
    TEVector2 pos = body->Position;
    const auto &s = body->Shape;

    if (s.type == CollisionType::AABB)
    {
        outPoints.Add(pos + s.aabb.min);
        outPoints.Add(pos + TEVector2{s.aabb.max.x, s.aabb.min.y});
        outPoints.Add(pos + s.aabb.max);
        outPoints.Add(pos + TEVector2{s.aabb.min.x, s.aabb.max.y});
    }
    else if (s.type == CollisionType::Triangle)
    {
        outPoints.Add(pos + s.triangle.points[0]);
        outPoints.Add(pos + s.triangle.points[1]);
        outPoints.Add(pos + s.triangle.points[2]);
    }
    else if (s.type == CollisionType::Polygon)
    {
        for (size_t i = 0; i < s.polygon.points.Num(); ++i)
            outPoints.Add(pos + s.polygon.points[i]);
    }
}

TESpatialHitResult PhysicsWorld::SweepLine(const TEVector2 &start, const TEVector2 &end)
{
    auto results = MultiSweepLine(start, end);
    if (!results.IsEmpty())
        return results[0];
    return TESpatialHitResult{};
}

TEArray<TESpatialHitResult> PhysicsWorld::MultiSweepLine(const TEVector2 &start, const TEVector2 &end)
{
    TEArray<TESpatialHitResult> hits;
    TEVector2 delta = end - start;
    float totalDist = std::sqrt(delta.x * delta.x + delta.y * delta.y);

    for (size_t bIdx = 0; bIdx < m_Bodies.Num(); ++bIdx)
    {
        RigidBody *body = m_Bodies[bIdx];
        if (!body)
            continue;

        TESpatialHitResult hit;
        hit.EntityID = body->m_VeloxEntityID;
        hit.EntityName = body->EntityName;
        hit.EntityTag = body->EntityTag;
        hit.Component = body->Component;
        const auto &s = body->Shape;

        if (s.type == CollisionType::Circle)
        {
            float t = 0.0f;
            TEVector2 normal;
            bool penetrating = false;
            if (RaycastCircle(start, delta, body->Position + s.circle.center, s.circle.radius, t, normal, penetrating))
            {
                hit.Hit = true;
                hit.Fraction = t;
                hit.Distance = t * totalDist;
                hit.Point = start + delta * t;
                hit.Normal = normal;
                hit.StartPenetrating = penetrating;
                hits.Add(hit);
            }
        }
        else
        {
            TEArray<TEVector2> pts;
            GetBodyPolygonPoints(body, pts);
            float minT = 2.0f;
            TEVector2 bestNorm;
            for (size_t i = 0; i < pts.Num(); ++i)
            {
                size_t next = (i + 1) % pts.Num();
                float t = 0.0f;
                TEVector2 norm;
                if (RaycastSegment(start, delta, pts[i], pts[next], t, norm))
                {
                    if (t < minT)
                    {
                        minT = t;
                        bestNorm = norm;
                    }
                }
            }
            if (minT <= 1.0f)
            {
                hit.Hit = true;
                hit.Fraction = minT;
                hit.Distance = minT * totalDist;
                hit.Point = start + delta * minT;
                hit.Normal = bestNorm;
                hits.Add(hit);
            }
        }
    }

    std::sort(hits.begin(), hits.end(),
              [](const TESpatialHitResult &a, const TESpatialHitResult &b) { return a.Fraction < b.Fraction; });

    return hits;
}

TESpatialHitResult PhysicsWorld::SweepCircle(const TEVector2 &start, const TEVector2 &end, const TEVector2 &radius,
                                             float angleRadians)
{
    auto results = MultiSweepCircle(start, end, radius, angleRadians);
    if (!results.IsEmpty())
        return results[0];
    return TESpatialHitResult{};
}

TEArray<TESpatialHitResult> PhysicsWorld::MultiSweepCircle(const TEVector2 &start, const TEVector2 &end,
                                                           const TEVector2 &radius, float angleRadians)
{
    TEArray<TESpatialHitResult> hits;
    TEVector2 delta = end - start;
    float totalDist = std::sqrt(delta.x * delta.x + delta.y * delta.y);
    float avgRadius = (radius.x + radius.y) * 0.5f;

    for (size_t bIdx = 0; bIdx < m_Bodies.Num(); ++bIdx)
    {
        RigidBody *body = m_Bodies[bIdx];
        if (!body)
            continue;

        TESpatialHitResult hit;
        hit.EntityID = body->m_VeloxEntityID;
        hit.EntityName = body->EntityName;
        hit.EntityTag = body->EntityTag;
        hit.Component = body->Component;
        const auto &s = body->Shape;

        if (s.type == CollisionType::Circle)
        {
            float t = 0.0f;
            TEVector2 normal;
            bool penetrating = false;
            float combRadius = s.circle.radius + avgRadius;
            if (RaycastCircle(start, delta, body->Position + s.circle.center, combRadius, t, normal, penetrating))
            {
                hit.Hit = true;
                hit.Fraction = t;
                hit.Distance = t * totalDist;
                hit.Point = start + delta * t - normal * avgRadius;
                hit.Normal = normal;
                hit.StartPenetrating = penetrating;
                hits.Add(hit);
            }
        }
        else
        {
            TEArray<TEVector2> pts;
            GetBodyPolygonPoints(body, pts);
            float minT = 2.0f;
            TEVector2 bestNorm;

            for (size_t i = 0; i < pts.Num(); ++i)
            {
                size_t next = (i + 1) % pts.Num();
                TEVector2 edge = pts[next] - pts[i];
                float elen = std::sqrt(edge.x * edge.x + edge.y * edge.y);
                if (elen > 1e-6f)
                {
                    TEVector2 enorm = {-edge.y / elen, edge.x / elen};
                    TEVector2 offsetA = pts[i] + enorm * avgRadius;
                    TEVector2 offsetB = pts[next] + enorm * avgRadius;

                    float t = 0.0f;
                    TEVector2 norm;
                    if (RaycastSegment(start, delta, offsetA, offsetB, t, norm))
                    {
                        if (t < minT)
                        {
                            minT = t;
                            bestNorm = norm;
                        }
                    }
                }

                float t = 0.0f;
                TEVector2 norm;
                bool pen = false;
                if (RaycastCircle(start, delta, pts[i], avgRadius, t, norm, pen))
                {
                    if (t < minT)
                    {
                        minT = t;
                        bestNorm = norm;
                    }
                }
            }

            if (minT <= 1.0f)
            {
                hit.Hit = true;
                hit.Fraction = minT;
                hit.Distance = minT * totalDist;
                hit.Point = start + delta * minT - bestNorm * avgRadius;
                hit.Normal = bestNorm;
                hits.Add(hit);
            }
        }
    }

    std::sort(hits.begin(), hits.end(),
              [](const TESpatialHitResult &a, const TESpatialHitResult &b) { return a.Fraction < b.Fraction; });

    return hits;
}

TESpatialHitResult PhysicsWorld::SweepBox(const TEVector2 &start, const TEVector2 &end, const TEVector2 &halfExtents,
                                          float angleRadians)
{
    auto results = MultiSweepBox(start, end, halfExtents, angleRadians);
    if (!results.IsEmpty())
        return results[0];
    return TESpatialHitResult{};
}

TEArray<TESpatialHitResult> PhysicsWorld::MultiSweepBox(const TEVector2 &start, const TEVector2 &end,
                                                        const TEVector2 &halfExtents, float angleRadians)
{
    TEArray<TESpatialHitResult> hits;
    TEVector2 delta = end - start;
    float totalDist = std::sqrt(delta.x * delta.x + delta.y * delta.y);

    // Construct oriented box vertices relative to center
    TEVector2 localVerts[4] = {RotateVec2({-halfExtents.x, -halfExtents.y}, angleRadians),
                               RotateVec2({halfExtents.x, -halfExtents.y}, angleRadians),
                               RotateVec2({halfExtents.x, halfExtents.y}, angleRadians),
                               RotateVec2({-halfExtents.x, halfExtents.y}, angleRadians)};

    for (size_t bIdx = 0; bIdx < m_Bodies.Num(); ++bIdx)
    {
        RigidBody *body = m_Bodies[bIdx];
        if (!body)
            continue;

        TESpatialHitResult hit;
        hit.EntityID = body->m_VeloxEntityID;
        hit.EntityName = body->EntityName;
        hit.EntityTag = body->EntityTag;
        hit.Component = body->Component;
        const auto &s = body->Shape;
        float minT = 2.0f;
        TEVector2 bestNorm;

        if (s.type == CollisionType::Circle)
        {
            // Sweep all 4 box edges against target circle
            for (int vi = 0; vi < 4; ++vi)
            {
                TEVector2 vStart = start + localVerts[vi];
                float t = 0.0f;
                TEVector2 norm;
                bool pen = false;
                if (RaycastCircle(vStart, delta, body->Position + s.circle.center, s.circle.radius, t, norm, pen))
                {
                    if (t < minT)
                    {
                        minT = t;
                        bestNorm = norm;
                    }
                }
            }
        }
        else
        {
            TEArray<TEVector2> targetPts;
            GetBodyPolygonPoints(body, targetPts);

            // 1. Sweep box corner vertices against target polygon edges
            for (int vi = 0; vi < 4; ++vi)
            {
                TEVector2 vStart = start + localVerts[vi];
                for (size_t ti = 0; ti < targetPts.Num(); ++ti)
                {
                    size_t next = (ti + 1) % targetPts.Num();
                    float t = 0.0f;
                    TEVector2 norm;
                    if (RaycastSegment(vStart, delta, targetPts[ti], targetPts[next], t, norm))
                    {
                        if (t < minT)
                        {
                            minT = t;
                            bestNorm = norm;
                        }
                    }
                }
            }

            // 2. Reverse sweep: target polygon vertices against swept box edges
            for (size_t ti = 0; ti < targetPts.Num(); ++ti)
            {
                TEVector2 revDelta = -delta;
                for (int bi = 0; bi < 4; ++bi)
                {
                    int bNext = (bi + 1) % 4;
                    TEVector2 bEdgeA = start + localVerts[bi];
                    TEVector2 bEdgeB = start + localVerts[bNext];

                    float t = 0.0f;
                    TEVector2 norm;
                    if (RaycastSegment(targetPts[ti], revDelta, bEdgeA, bEdgeB, t, norm))
                    {
                        if (t < minT)
                        {
                            minT = t;
                            bestNorm = -norm;
                        }
                    }
                }
            }
        }

        if (minT <= 1.0f)
        {
            hit.Hit = true;
            hit.Fraction = minT;
            hit.Distance = minT * totalDist;
            hit.Point = start + delta * minT;
            hit.Normal = bestNorm;
            hits.Add(hit);
        }
    }

    std::sort(hits.begin(), hits.end(),
              [](const TESpatialHitResult &a, const TESpatialHitResult &b) { return a.Fraction < b.Fraction; });

    return hits;
}

TESpatialHitResult PhysicsWorld::SweepCustom(const TEVector2 &start, const TEVector2 &end, const CollisionShape &shape,
                                             float angleRadians)
{
    auto results = MultiSweepCustom(start, end, shape, angleRadians);
    if (!results.IsEmpty())
        return results[0];
    return TESpatialHitResult{};
}

TEArray<TESpatialHitResult> PhysicsWorld::MultiSweepCustom(const TEVector2 &start, const TEVector2 &end,
                                                           const CollisionShape &shape, float angleRadians)
{
    if (shape.type == CollisionType::Circle)
    {
        return MultiSweepCircle(start, end, TEVector2(shape.circle.radius, shape.circle.radius), angleRadians);
    }
    else if (shape.type == CollisionType::AABB)
    {
        TEVector2 halfExtents = (shape.aabb.max - shape.aabb.min) * 0.5f;
        return MultiSweepBox(start, end, halfExtents, angleRadians);
    }

    // Dynamic polygon / triangle custom shape sweep
    TEArray<TEVector2> customVerts;
    if (shape.type == CollisionType::Triangle)
    {
        customVerts.Add(RotateVec2(shape.triangle.points[0], angleRadians));
        customVerts.Add(RotateVec2(shape.triangle.points[1], angleRadians));
        customVerts.Add(RotateVec2(shape.triangle.points[2], angleRadians));
    }
    else if (shape.type == CollisionType::Polygon)
    {
        for (size_t i = 0; i < shape.polygon.points.Num(); ++i)
        {
            customVerts.Add(RotateVec2(shape.polygon.points[i], angleRadians));
        }
    }

    if (customVerts.IsEmpty())
    {
        return MultiSweepLine(start, end);
    }

    TEArray<TESpatialHitResult> hits;
    TEVector2 delta = end - start;
    float totalDist = std::sqrt(delta.x * delta.x + delta.y * delta.y);

    for (size_t bIdx = 0; bIdx < m_Bodies.Num(); ++bIdx)
    {
        RigidBody *body = m_Bodies[bIdx];
        if (!body)
            continue;

        TESpatialHitResult hit;
        hit.EntityID = body->m_VeloxEntityID;
        hit.EntityName = body->EntityName;
        hit.EntityTag = body->EntityTag;
        hit.Component = body->Component;
        float minT = 2.0f;
        TEVector2 bestNorm;

        TEArray<TEVector2> targetPts;
        GetBodyPolygonPoints(body, targetPts);

        // 1. Sweep custom shape vertices against target polygon edges
        for (size_t vi = 0; vi < customVerts.Num(); ++vi)
        {
            TEVector2 vStart = start + customVerts[vi];
            for (size_t ti = 0; ti < targetPts.Num(); ++ti)
            {
                size_t next = (ti + 1) % targetPts.Num();
                float t = 0.0f;
                TEVector2 norm;
                if (RaycastSegment(vStart, delta, targetPts[ti], targetPts[next], t, norm))
                {
                    if (t < minT)
                    {
                        minT = t;
                        bestNorm = norm;
                    }
                }
            }
        }

        // 2. Reverse sweep: target polygon vertices against swept custom shape edges
        for (size_t ti = 0; ti < targetPts.Num(); ++ti)
        {
            TEVector2 revDelta = -delta;
            for (size_t ci = 0; ci < customVerts.Num(); ++ci)
            {
                size_t cNext = (ci + 1) % customVerts.Num();
                TEVector2 cEdgeA = start + customVerts[ci];
                TEVector2 cEdgeB = start + customVerts[cNext];

                float t = 0.0f;
                TEVector2 norm;
                if (RaycastSegment(targetPts[ti], revDelta, cEdgeA, cEdgeB, t, norm))
                {
                    if (t < minT)
                    {
                        minT = t;
                        bestNorm = -norm;
                    }
                }
            }
        }

        if (minT <= 1.0f)
        {
            hit.Hit = true;
            hit.Fraction = minT;
            hit.Distance = minT * totalDist;
            hit.Point = start + delta * minT;
            hit.Normal = bestNorm;
            hits.Add(hit);
        }
    }

    std::sort(hits.begin(), hits.end(),
              [](const TESpatialHitResult &a, const TESpatialHitResult &b) { return a.Fraction < b.Fraction; });

    return hits;
}
