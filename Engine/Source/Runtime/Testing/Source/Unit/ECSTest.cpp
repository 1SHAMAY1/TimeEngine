#include "PreRequisites.h"
#include "EntityManager.hpp"
#include "Scene.hpp"
#include "TagComponent.hpp"
#include "TransformComponent.hpp"
#include "TestHarness.hpp"
#include "TestRegistry.hpp"

TE_TEST_CASE(ECS, EntityCreationAndDestruction)
{
    auto scene = CreateRef<Scene>("TestScene");
    TE_CHECK(scene != nullptr);

    Entity e1 = scene->CreateEntity("Player");
    TE_CHECK(e1.IsValid());
    TE_CHECK_EQ(e1.GetID(), 1);

    Entity e2 = scene->CreateEntity("Enemy");
    TE_CHECK(e2.IsValid());
    TE_CHECK_EQ(e2.GetID(), 2);
    TE_CHECK_NE(e1, e2);

    scene->DestroyEntity(e1);
    TE_CHECK(!e1.IsValid());
    TE_CHECK(e2.IsValid());
}

TE_TEST_CASE(ECS, ComponentAttachmentAndQuery)
{
    auto scene = CreateRef<Scene>("ComponentScene");
    Entity entity = scene->CreateEntity("TestActor");

    auto *tagComp = entity.GetComponent<TagComponent>();
    TE_CHECK(tagComp != nullptr);
    TE_CHECK_EQ(tagComp->Tag, "TestActor");

    auto *transformComp = entity.GetComponent<TransformComponent>();
    TE_CHECK(transformComp != nullptr);

    transformComp->Transform.Position = TEVector(10.0f, 20.0f, 30.0f);
    TE_CHECK_EQ(transformComp->Transform.Position.x, 10.0f);
    TE_CHECK_EQ(transformComp->Transform.Position.y, 20.0f);
    TE_CHECK_EQ(transformComp->Transform.Position.z, 30.0f);

    TE_CHECK(entity.HasComponent<TagComponent>());
    TE_CHECK(entity.HasComponent<TransformComponent>());

    entity.RemoveComponent<TagComponent>();
    TE_CHECK(!entity.HasComponent<TagComponent>());
    TE_CHECK(entity.GetComponent<TagComponent>() == nullptr);
}
