#include "PreRequisites.h"
#include "PhysicsWorld.hpp"
#include "TestHarness.hpp"
#include "TestRegistry.hpp"

TE_TEST_CASE(Physics, HeadlessSimulationAndGravity)
{
    PhysicsWorld world;
    world.SetGravity(TEVector2(0.0f, -9.8f));
    TE_CHECK_EQ(world.GetGravity().y, -9.8f);

    RigidBody body;
    body.Position = TEVector2(0.0f, 100.0f);
    body.Velocity = TEVector2(0.0f, 0.0f);
    body.Mass = 1.0f;
    body.InverseMass = 1.0f;
    body.IsStatic = false;
    body.ApplyForce(TEVector2(0.0f, -980.0f));

    body.Integrate(1.0f / 60.0f);

    TE_CHECK(body.Position.y < 100.0f);
    TE_CHECK(body.Velocity.y < 0.0f);
}

TE_TEST_CASE(Physics, StaticBodyImmobility)
{
    PhysicsWorld world;
    world.SetGravity(TEVector2(0.0f, -9.8f));

    RigidBody staticBody;
    staticBody.Position = TEVector2(5.0f, 5.0f);
    staticBody.Velocity = TEVector2(0.0f, 0.0f);
    staticBody.IsStatic = true;
    staticBody.InverseMass = 0.0f;
    staticBody.ApplyForce(TEVector2(0.0f, -980.0f));

    staticBody.Integrate(1.0f / 60.0f);

    TE_CHECK_EQ(staticBody.Position.x, 5.0f);
    TE_CHECK_EQ(staticBody.Position.y, 5.0f);
    TE_CHECK_EQ(staticBody.Velocity.x, 0.0f);
    TE_CHECK_EQ(staticBody.Velocity.y, 0.0f);
}
