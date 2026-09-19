#include "PreRequisites.h"
#include "GameplayUtils.hpp"
#include "Application.hpp"
#include "Plugin/PluginManager.hpp"
#include "EntityManager.hpp"
#include "Scene.hpp"
#include "TagComponent.hpp"
#include "TransformComponent.hpp"
#include "TComponent.hpp"
#include "ControllableGameObject.hpp"
#include "UIAsset.hpp"
#include "PhysicsWorld.hpp"
#include "Renderer2D.hpp"

#include <algorithm>
#include <sstream>

TEString GameplayUtils::GetEntityDisplayName(EntityManager &mgr, Entity entity)
{
    if (auto *tag = mgr.GetComponent<TagComponent>(entity))
    {
        if (!tag->Tag.empty())
            return tag->Tag;
    }
    return TEString("Entity ") + TEString::FromInt64((uint64_t)entity);
}

TEMatrix4 GameplayUtils::ResolveWorldTransform(EntityManager &mgr, Entity entity, TComponent *comp)
{
    // Accumulate entity world transform by walking up the entity parent chain
    TEMatrix4 entityWorldModel(1.0f);
    TEArray<TEMatrix4> entityMatrices;

    Entity currEntity = entity;
    int depthGuard = 0;
    while (currEntity.IsValid() && depthGuard++ < 64)
    {
        if (auto *tc = mgr.GetComponent<TransformComponent>(currEntity))
        {
            entityMatrices.push_back(tc->Transform.GetMatrix());
            if (tc->Parent != 0 && mgr.IsValid(tc->Parent))
            {
                currEntity = Entity(tc->Parent, &mgr);
            }
            else
            {
                break;
            }
        }
        else
        {
            break;
        }
    }

    std::reverse(entityMatrices.begin(), entityMatrices.end());
    for (const auto &mat : entityMatrices)
    {
        entityWorldModel = entityWorldModel * mat;
    }

    // Walk the component parent chain
    TEArray<TComponent *> chain;
    TComponent *curr = comp;
    while (curr)
    {
        chain.push_back(curr);
        curr = curr->GetParentComponent();
    }
    std::reverse(chain.begin(), chain.end());

    TEMatrix4 model = entityWorldModel;
    for (auto *node : chain)
        model = model * node->Transform.GetMatrix();

    return model;
}

bool GameplayUtils::EntityContainsPoint(EntityManager &mgr, Entity entity, const TEVector2 &worldPoint)
{
    auto allComponents = mgr.GetAllComponents(entity);
    for (auto *comp : allComponents)
    {
        TEMatrix4 model = ResolveWorldTransform(mgr, entity, comp);
        if (comp->ContainsPoint(model, worldPoint))
        {
            TE_CORE_INFO("[PickEntity] Hit component '{0}' on entity '{1}'", comp->GetClassName(),
                         GetEntityDisplayName(mgr, entity));
            return true;
        }
    }

    // Fallback: proximity check on transform origin
    if (auto *transform = mgr.GetComponent<TransformComponent>(entity))
    {
        TEMatrix4 worldModel = ResolveWorldTransform(mgr, entity, nullptr);
        TEVector2 pos = {worldModel[3][0], worldModel[3][1]};
        float d = Distance(worldPoint, pos);
        if (d <= 0.5f)
        {
            TE_CORE_INFO("[PickEntity] Hit proximity (dist={0}) on entity '{1}'", d, GetEntityDisplayName(mgr, entity));
            return true;
        }
    }

    return false;
}

Entity GameplayUtils::PickEntity(Scene &scene, const TEVector2 &worldPoint)
{
    auto &entityManager = scene.GetEntityManager();
    const auto &alive = entityManager.GetAliveEntities();

    Entity result; // invalid by default
    for (auto id : alive)
    {
        Entity e(id, &entityManager);
        if (EntityContainsPoint(entityManager, e, worldPoint))
            result = e; // last hit wins (topmost drawn last)
    }
    return result;
}

TEVector2 GameplayUtils::ViewportPixelToWorld(TEVector2 pixelPos, TEVector2 viewportSize, TEVector2 cameraPos,
                                              float cameraZoom)
{
    float aspect = (viewportSize.y > 0) ? viewportSize.x / viewportSize.y : 1.0f;
    float vpW = (viewportSize.x > 0) ? viewportSize.x : 1.0f;
    float vpH = (viewportSize.y > 0) ? viewportSize.y : 1.0f;

    TEVector2 world;
    world.x = ((pixelPos.x / vpW) * 2.0f - 1.0f) * aspect * cameraZoom + cameraPos.x;
    world.y = (1.0f - (pixelPos.y / vpH) * 2.0f) * cameraZoom + cameraPos.y;
    return world;
}

TEVector2 GameplayUtils::WorldToViewportPixel(TEVector2 worldPos, TEVector2 viewportSize, TEVector2 cameraPos,
                                              float cameraZoom)
{
    float aspect = (viewportSize.y > 0) ? viewportSize.x / viewportSize.y : 1.0f;
    float vpW = (viewportSize.x > 0) ? viewportSize.x : 1.0f;
    float vpH = (viewportSize.y > 0) ? viewportSize.y : 1.0f;

    float denomX = (aspect * cameraZoom);
    if (denomX == 0.0f)
        denomX = 1.0f;
    float denomY = cameraZoom;
    if (denomY == 0.0f)
        denomY = 1.0f;

    TEVector2 pixel;
    pixel.x = (((worldPos.x - cameraPos.x) / denomX) + 1.0f) * 0.5f * vpW;
    pixel.y = (1.0f - ((worldPos.y - cameraPos.y) / denomY)) * 0.5f * vpH;
    return pixel;
}

Entity GameplayUtils::SpawnEntity(Scene &scene, const TEString &name, const TEVector2 &position)
{
    Entity entity = scene.CreateEntity(name);
    auto *transform = entity.GetComponent<TransformComponent>();
    if (transform)
    {
        transform->Transform.Position.x = position.x;
        transform->Transform.Position.y = position.y;
    }
    return entity;
}

void GameplayUtils::Destroy(Scene &scene, Entity entity)
{
    if (entity.IsValid())
    {
        scene.DestroyEntity(entity);
    }
}

void GameplayUtils::QuitGame() { Application::Get().Close(); }

Entity GameplayUtils::SpawnControllableGameObject(Scene &scene, const TEString &name, const TEVector2 &position)
{
    Entity entity = scene.CreateEntity(name);
    auto *transform = entity.GetComponent<TransformComponent>();
    if (transform)
    {
        transform->Transform.Position.x = position.x;
        transform->Transform.Position.y = position.y;
    }

    auto *controllable = entity.AddComponent<ControllableGameObject>();
    if (controllable)
    {
        controllable->EnableInput();
    }
    return entity;
}

TERef<UIWidget> GameplayUtils::CreateWidget(const TEString &uiAssetPath)
{
    UIAsset asset;
    if (asset.LoadFromFile(uiAssetPath))
    {
        return asset.InstantiateWidgetTree();
    }
    return nullptr;
}

namespace
{
static void DrawDebugSweepInternal(const TEVector2 &start, const TEVector2 &end, const TESpatialHitResult &hit,
                                   const TEColor &traceColor, const TEColor &hitColor)
{
    // If future persistent duration debug system is wired, debugDuration can register timer;
    // for direct frame debug, submit lines to active Renderer2D if available or active pipeline.
    (void)traceColor;
    (void)hitColor;
    (void)start;
    (void)end;
    (void)hit;
}
} // namespace

TESpatialHitResult GameplayUtils::SweepLine(const TEVector2 &start, const TEVector2 &end, Scene *scene, bool drawDebug,
                                            float debugDuration, const TEColor &traceColor, const TEColor &hitColor)
{
    TESpatialHitResult result{};
    if (scene && scene->GetPhysicsWorld())
    {
        result = scene->GetPhysicsWorld()->SweepLine(start, end);
    }
    if (drawDebug)
    {
        (void)debugDuration;
        DrawDebugSweepInternal(start, end, result, traceColor, hitColor);
    }
    return result;
}

TEArray<TESpatialHitResult> GameplayUtils::MultiSweepLine(const TEVector2 &start, const TEVector2 &end, Scene *scene,
                                                          bool drawDebug, float debugDuration,
                                                          const TEColor &traceColor, const TEColor &hitColor)
{
    TEArray<TESpatialHitResult> results{};
    if (scene && scene->GetPhysicsWorld())
    {
        results = scene->GetPhysicsWorld()->MultiSweepLine(start, end);
    }
    if (drawDebug)
    {
        (void)debugDuration;
        for (const auto &hit : results)
        {
            DrawDebugSweepInternal(start, end, hit, traceColor, hitColor);
        }
    }
    return results;
}

TESpatialHitResult GameplayUtils::SweepCircle(const TEVector2 &start, const TEVector2 &end, const TEVector2 &radius,
                                              float angleRadians, Scene *scene, bool drawDebug, float debugDuration,
                                              const TEColor &traceColor, const TEColor &hitColor)
{
    TESpatialHitResult result{};
    if (scene && scene->GetPhysicsWorld())
    {
        result = scene->GetPhysicsWorld()->SweepCircle(start, end, radius, angleRadians);
    }
    if (drawDebug)
    {
        (void)debugDuration;
        DrawDebugSweepInternal(start, end, result, traceColor, hitColor);
    }
    return result;
}

TEArray<TESpatialHitResult> GameplayUtils::MultiSweepCircle(const TEVector2 &start, const TEVector2 &end,
                                                            const TEVector2 &radius, float angleRadians, Scene *scene,
                                                            bool drawDebug, float debugDuration,
                                                            const TEColor &traceColor, const TEColor &hitColor)
{
    TEArray<TESpatialHitResult> results{};
    if (scene && scene->GetPhysicsWorld())
    {
        results = scene->GetPhysicsWorld()->MultiSweepCircle(start, end, radius, angleRadians);
    }
    if (drawDebug)
    {
        (void)debugDuration;
        for (const auto &hit : results)
        {
            DrawDebugSweepInternal(start, end, hit, traceColor, hitColor);
        }
    }
    return results;
}

TESpatialHitResult GameplayUtils::SweepBox(const TEVector2 &start, const TEVector2 &end, const TEVector2 &halfExtents,
                                           float angleRadians, Scene *scene, bool drawDebug, float debugDuration,
                                           const TEColor &traceColor, const TEColor &hitColor)
{
    TESpatialHitResult result{};
    if (scene && scene->GetPhysicsWorld())
    {
        result = scene->GetPhysicsWorld()->SweepBox(start, end, halfExtents, angleRadians);
    }
    if (drawDebug)
    {
        (void)debugDuration;
        DrawDebugSweepInternal(start, end, result, traceColor, hitColor);
    }
    return result;
}

TEArray<TESpatialHitResult> GameplayUtils::MultiSweepBox(const TEVector2 &start, const TEVector2 &end,
                                                         const TEVector2 &halfExtents, float angleRadians, Scene *scene,
                                                         bool drawDebug, float debugDuration, const TEColor &traceColor,
                                                         const TEColor &hitColor)
{
    TEArray<TESpatialHitResult> results{};
    if (scene && scene->GetPhysicsWorld())
    {
        results = scene->GetPhysicsWorld()->MultiSweepBox(start, end, halfExtents, angleRadians);
    }
    if (drawDebug)
    {
        (void)debugDuration;
        for (const auto &hit : results)
        {
            DrawDebugSweepInternal(start, end, hit, traceColor, hitColor);
        }
    }
    return results;
}

TESpatialHitResult GameplayUtils::SweepCustom(const TEVector2 &start, const TEVector2 &end, const CollisionShape &shape,
                                              float angleRadians, Scene *scene, bool drawDebug, float debugDuration,
                                              const TEColor &traceColor, const TEColor &hitColor)
{
    TESpatialHitResult result{};
    if (scene && scene->GetPhysicsWorld())
    {
        result = scene->GetPhysicsWorld()->SweepCustom(start, end, shape, angleRadians);
    }
    if (drawDebug)
    {
        (void)debugDuration;
        DrawDebugSweepInternal(start, end, result, traceColor, hitColor);
    }
    return result;
}

TEArray<TESpatialHitResult> GameplayUtils::MultiSweepCustom(const TEVector2 &start, const TEVector2 &end,
                                                            const CollisionShape &shape, float angleRadians,
                                                            Scene *scene, bool drawDebug, float debugDuration,
                                                            const TEColor &traceColor, const TEColor &hitColor)
{
    TEArray<TESpatialHitResult> results{};
    if (scene && scene->GetPhysicsWorld())
    {
        results = scene->GetPhysicsWorld()->MultiSweepCustom(start, end, shape, angleRadians);
    }
    if (drawDebug)
    {
        (void)debugDuration;
        for (const auto &hit : results)
        {
            DrawDebugSweepInternal(start, end, hit, traceColor, hitColor);
        }
    }
    return results;
}
