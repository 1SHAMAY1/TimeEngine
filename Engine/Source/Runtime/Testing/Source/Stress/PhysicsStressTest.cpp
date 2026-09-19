#include "PreRequisites.h"
#include "PhysicsWorld.hpp"
#include "TestHarness.hpp"
#include "TestRegistry.hpp"

TE_STRESS_TEST_CASE(Stress, PhysicsRigidBodiesBenchmark)
{
    const size_t bodyCount = 1000;
    PhysicsWorld world;
    world.SetGravity(TEVector2(0.0f, 980.0f));

    // 1. Add static floor body to test collision resolution and broadphase stacking
    RigidBody floor;
    floor.Position = TEVector2(640.0f, 700.0f);
    floor.Mass = 0.0f;
    floor.InverseMass = 0.0f;
    floor.IsStatic = true;
    floor.Shape.type = CollisionType::AABB;
    floor.Shape.aabb.min = TEVector2(-2000.0f, -20.0f);
    floor.Shape.aabb.max = TEVector2(2000.0f, 20.0f);
    world.AddBody(&floor);

    // 2. Add dense dynamic boxes
    TEArray<RigidBody> bodies;
    const int cols = 40;
    for (size_t i = 0; i < bodyCount; ++i)
    {
        RigidBody b;
        float posX = 200.0f + static_cast<float>(i % cols) * 22.0f;
        float posY = 100.0f + static_cast<float>(i / cols) * 22.0f;
        b.Position = TEVector2(posX, posY);
        b.Velocity = TEVector2(0.0f, 0.0f);
        b.Mass = 1.0f;
        b.InverseMass = 1.0f;
        b.Restitution = 0.05f;
        b.StaticFriction = 0.8f;
        b.DynamicFriction = 0.5f;
        b.IsStatic = false;
        b.Shape.type = CollisionType::AABB;
        b.Shape.aabb.min = TEVector2(-8.0f, -8.0f);
        b.Shape.aabb.max = TEVector2(8.0f, 8.0f);
        bodies.Add(b);
    }

    for (size_t i = 0; i < bodyCount; ++i)
    {
        world.AddBody(&bodies[i]);
    }

    // Warmup 5 ticks
    for (size_t w = 0; w < 5; ++w)
    {
        world.Step(1.0f / 60.0f);
    }

    const size_t dynamicSteps = 60;
    TE_BENCHMARK("Physics Dynamic Collisions (1,000 Bodies - 60 ticks)", dynamicSteps, [&]() {
        for (size_t step = 0; step < dynamicSteps; ++step)
        {
            world.Step(1.0f / 60.0f);
        }
    });

    TE_CHECK(bodies[0].Position.y <= 700.0f);

    // Let bodies complete fall and settle on ground (240 ticks)
    for (size_t s = 0; s < 240; ++s)
    {
        world.Step(1.0f / 60.0f);
    }

    const size_t sleepingSteps = 60;
    TE_BENCHMARK("Physics Settled Sleep Islands (1,000 Bodies - 60 ticks)", sleepingSteps, [&]() {
        for (size_t step = 0; step < sleepingSteps; ++step)
        {
            world.Step(1.0f / 60.0f);
        }
    });

    // Test wake propagation on contact
    world.WakeTouching(bodies[0].m_VeloxEntityID);
    world.Step(1.0f / 60.0f);

    for (size_t i = 0; i < bodyCount; ++i)
    {
        world.RemoveBody(&bodies[i]);
    }
    world.RemoveBody(&floor);
}
