#include "PreRequisites.h"
#include "EntityManager.hpp"
#include "Scene.hpp"
#include "TagComponent.hpp"
#include "TransformComponent.hpp"
#include "GameplayUtils.hpp"
#include "TestHarness.hpp"
#include "TestRegistry.hpp"

TE_STRESS_TEST_CASE(Stress, EntityScaleMillionBenchmark)
{
    const size_t entityCount = 200000; // 200k entities for fast stress pass, scalable to 1M+
    auto scene = CreateRef<Scene>("StressScene");

    TE_BENCHMARK("Entity Allocation (200k)", entityCount, [&]() {
        for (size_t i = 0; i < entityCount; ++i)
        {
            Entity e = scene->CreateEntity("Entity_" + TEString::FromInt(static_cast<int>(i)));
            auto *transform = e.GetComponent<TransformComponent>();
            if (transform)
            {
                transform->Transform.Position = TEVector(static_cast<float>(i), 0.0f, 0.0f);
            }
        }
    });

    TE_CHECK_EQ(scene->GetEntityManager().GetAliveEntities().size(), entityCount);

    TE_BENCHMARK("Transform Mutation (200k)", entityCount, [&]() {
        for (EntityID id : scene->GetEntityManager().GetAliveEntities())
        {
            auto *transform = scene->GetEntityManager().GetComponent<TransformComponent>(id);
            if (transform)
            {
                transform->Transform.Position.y += 1.0f;
            }
        }
    });

    TE_BENCHMARK("Entity Destruction (200k)", entityCount, [&]() {
        auto alive = scene->GetEntityManager().GetAliveEntities();
        for (EntityID id : alive)
        {
            scene->DestroyEntity(Entity(id, &scene->GetEntityManager()));
        }
    });

    TE_CHECK_EQ(scene->GetEntityManager().GetAliveEntities().size(), 0);
}

TE_STRESS_TEST_CASE(Stress, GameplayUtilsContinuousSpawnDestroyChurn)
{
    const size_t iterations = 50000;
    auto scene = CreateRef<Scene>("ChurnStressScene");

    TE_BENCHMARK("Spawn & Destroy Churn (50k Iterations)", iterations, [&]() {
        for (size_t i = 0; i < iterations; ++i)
        {
            // Spawn entity with transform via GameplayUtils
            Entity spawned = GameplayUtils::SpawnEntity(*scene, "ChurnActor", {static_cast<float>(i), 100.0f});
            
            // Immediately destroy to test memory allocator and archetype churn stability
            GameplayUtils::Destroy(*scene, spawned);
        }
    });

    TE_CHECK_EQ(scene->GetEntityManager().GetAliveEntities().size(), 0);
}

