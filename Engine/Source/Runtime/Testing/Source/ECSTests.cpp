#include "PreRequisites.h"
#include "TestHarness.hpp"
#include "TestRegistry.hpp"
#include "EntityManager.hpp"
#include "TransformComponent.hpp"
#include "TagComponent.hpp"
#include "SpriteComponent.hpp"
#include "ECS/ComponentTypeID.hpp"
#include "ECS/ComponentMask.hpp"
#include "ECS/Archetype.hpp"
#include "ECS/Query.hpp"
#include "ECS/SystemScheduler.hpp"

struct TestPositionComponent : public TComponent
{
    float X = 0.0f;
    float Y = 0.0f;
};

struct TestVelocityComponent : public TComponent
{
    float Vx = 1.0f;
    float Vy = 2.0f;
};

struct TestHeavyPayloadComponent : public TComponent
{
    uint8_t Data[1024];
    TestHeavyPayloadComponent()
    {
        for (size_t i = 0; i < 1024; ++i)
            Data[i] = static_cast<uint8_t>(i % 256);
    }
};

TE_TEST_CASE(PureECS, ComponentTypeIDAndMask)
{
    ComponentID id1 = ComponentTypeID::Get<TestPositionComponent>();
    ComponentID id2 = ComponentTypeID::Get<TestVelocityComponent>();
    ComponentID id1_again = ComponentTypeID::Get<TestPositionComponent>();

    TE_CHECK_NE(id1, id2);
    TE_CHECK_EQ(id1, id1_again);

    ComponentMask mask = MakeComponentMask<TestPositionComponent, TestVelocityComponent>();
    TE_CHECK(mask.test(id1));
    TE_CHECK(mask.test(id2));
    TE_CHECK_EQ(mask.count(), 2);
}

TE_TEST_CASE(PureECS, ArchetypeEntityCreationAndQuery)
{
    EntityManager em;

    Entity e1 = em.CreateEntity();
    auto *pos1 = e1.AddComponent<TestPositionComponent>();
    pos1->X = 10.0f;
    pos1->Y = 20.0f;

    auto *vel1 = e1.AddComponent<TestVelocityComponent>();
    vel1->Vx = 5.0f;
    vel1->Vy = 15.0f;

    Entity e2 = em.CreateEntity();
    auto *pos2 = e2.AddComponent<TestPositionComponent>();
    pos2->X = 100.0f;
    pos2->Y = 200.0f;

    TE_CHECK(e1.HasComponent<TestPositionComponent>());
    TE_CHECK(e1.HasComponent<TestVelocityComponent>());
    TE_CHECK(e2.HasComponent<TestPositionComponent>());
    TE_CHECK(!e2.HasComponent<TestVelocityComponent>());

    size_t posCount = 0;
    ComponentQuery<TestPositionComponent> qPos(em);
    qPos.ForEach([&](EntityID id, TestPositionComponent &pos) { posCount++; });
    TE_CHECK_EQ(posCount, 2);

    size_t posVelCount = 0;
    ComponentQuery<TestPositionComponent, TestVelocityComponent> qPosVel(em);
    qPosVel.ForEach(
        [&](EntityID id, TestPositionComponent &pos, TestVelocityComponent &vel)
        {
            pos.X += vel.Vx;
            pos.Y += vel.Vy;
            posVelCount++;
        });
    TE_CHECK_EQ(posVelCount, 1);
    auto *checkPos = e1.GetComponent<TestPositionComponent>();
    TE_CHECK(checkPos != nullptr);
    TE_CHECK_EQ(checkPos->X, 15.0f);
    TE_CHECK_EQ(checkPos->Y, 35.0f);
}

TE_TEST_CASE(PureECS, EntityRemovalAndSwapLast)
{
    EntityManager em;
    Entity e1 = em.CreateEntity();
    Entity e2 = em.CreateEntity();
    Entity e3 = em.CreateEntity();

    auto *p1 = e1.AddComponent<TestPositionComponent>();
    p1->X = 1.0f;
    auto *p2 = e2.AddComponent<TestPositionComponent>();
    p2->X = 2.0f;
    auto *p3 = e3.AddComponent<TestPositionComponent>();
    p3->X = 3.0f;

    TE_CHECK_EQ(em.GetAliveEntities().Num(), 3);

    em.DestroyEntity(e2);
    TE_CHECK_EQ(em.GetAliveEntities().Num(), 2);
    TE_CHECK(!em.IsValid(e2.GetID()));
    TE_CHECK(em.IsValid(e1.GetID()));
    TE_CHECK(em.IsValid(e3.GetID()));

    auto *checkP3 = e3.GetComponent<TestPositionComponent>();
    TE_CHECK(checkP3 != nullptr);
    TE_CHECK_EQ(checkP3->X, 3.0f);
}

TE_STRESS_TEST_CASE(PureECS, MassiveEntityStress100K)
{
    EntityManager em;
    constexpr size_t EntityCount = 100000;

    TEArray<Entity> entities;

    TE_BENCHMARK("Spawn 100k Entities with Position and Velocity", EntityCount,
                 (
                     [&]()
                     {
                         for (size_t i = 0; i < EntityCount; ++i)
                         {
                             Entity e = em.CreateEntityWith<TestPositionComponent, TestVelocityComponent>();
                             auto *pos = e.GetComponent<TestPositionComponent>();
                             if (pos)
                             {
                                 pos->X = static_cast<float>(i);
                                 pos->Y = static_cast<float>(i * 2);
                             }
                             auto *vel = e.GetComponent<TestVelocityComponent>();
                             if (vel)
                             {
                                 vel->Vx = 1.0f;
                                 vel->Vy = 1.0f;
                             }
                             entities.Add(e);
                         }
                     }));

    TE_CHECK_EQ(em.GetAliveEntities().Num(), EntityCount);

    TE_BENCHMARK("Query and Iterate 100k Entities in ECS", EntityCount,
                 (
                     [&]()
                     {
                         ComponentQuery<TestPositionComponent, TestVelocityComponent> q(em);
                         q.ForEach(
                             [](EntityID id, TestPositionComponent &pos, TestVelocityComponent &vel)
                             {
                                 pos.X += vel.Vx;
                                 pos.Y += vel.Vy;
                             });
                     }));

    TE_BENCHMARK("Destroy 100k Entities in ECS", EntityCount,
                 (
                     [&]()
                     {
                         for (auto &e : entities)
                         {
                             em.DestroyEntity(e);
                         }
                     }));

    TE_CHECK_EQ(em.GetAliveEntities().Num(), 0);
}

TE_TEST_CASE(PureECS, MemoryLeakAndHeavyPayloadIntegrity)
{
    EntityManager em;
    constexpr size_t BatchSize = 10000;

    for (int cycle = 0; cycle < 5; ++cycle)
    {
        TEArray<Entity> entities;

        for (size_t i = 0; i < BatchSize; ++i)
        {
            Entity e = em.CreateEntity();
            auto *heavy = e.AddComponent<TestHeavyPayloadComponent>();
            TE_CHECK_EQ(heavy->Data[0], 0);
            TE_CHECK_EQ(heavy->Data[255], 255);
            entities.Add(e);
        }

        TE_CHECK_EQ(em.GetAliveEntities().Num(), BatchSize);

        for (auto &e : entities)
        {
            em.DestroyEntity(e);
        }

        TE_CHECK_EQ(em.GetAliveEntities().Num(), 0);
    }
}

TE_TEST_CASE(PureECS, DeepEntityHierarchyNoStackOverflow)
{
    EntityManager em;
    constexpr size_t Depth = 10000;

    Entity parent = em.CreateEntity();
    parent.AddComponent<TransformComponent>();

    Entity current = parent;
    for (size_t i = 1; i < Depth; ++i)
    {
        Entity child = em.CreateEntity();
        auto *childTC = child.AddComponent<TransformComponent>();
        childTC->Parent = current.GetID();

        auto *parentTC = current.GetComponent<TransformComponent>();
        parentTC->Children.Add(child.GetID());

        current = child;
    }

    TE_CHECK_EQ(em.GetAliveEntities().Num(), Depth);

    size_t count = 0;
    Entity traverse = parent;
    while (traverse.IsValid())
    {
        count++;
        auto *tc = traverse.GetComponent<TransformComponent>();
        if (tc && !tc->Children.IsEmpty())
        {
            traverse = Entity(tc->Children[0], &em);
        }
        else
        {
            break;
        }
    }

    TE_CHECK_EQ(count, Depth);
}
