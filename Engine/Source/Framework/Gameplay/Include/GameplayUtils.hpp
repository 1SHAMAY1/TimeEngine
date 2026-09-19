#pragma once
#include "PreRequisites.h"
#include "TFunctionLibrary.hpp"
#include "TEColor.hpp"
#include "MathUtils.hpp"
#include "EntityManager.hpp"
#include <algorithm>
#include <functional>
#include <initializer_list>
#include <type_traits>

class Scene;
class Entity;
class EntityManager;
class TComponent;
class TObject;

// ==========================================
// Managed Memory Allocation & Lifetime Helpers
// ==========================================
namespace Memory
{
template <typename T, typename... Args> inline Scope<T> NewScope(Args &&...args)
{
    return CreateScope<T>(std::forward<Args>(args)...);
}

template <typename T, typename... Args> inline Ref<T> NewRef(Args &&...args)
{
    return CreateRef<T>(std::forward<Args>(args)...);
}
} // namespace Memory

// ==========================================
// Spatial Query & Sweep Result Structure
// ==========================================
struct TESpatialHitResult
{
    bool Hit = false;
    TEVector2 Point = {0.0f, 0.0f};
    TEVector2 Normal = {0.0f, 0.0f};
    float Distance = 0.0f;
    float Fraction = 1.0f;
    uint32_t EntityID = 0;
    TEString EntityName;
    TEString EntityTag;
    class TComponent *Component = nullptr;
    bool StartPenetrating = false;

    constexpr explicit operator bool() const noexcept { return Hit; }
};

// ==========================================
// GameplayUtils - Gameplay & Scene Utilities
// ==========================================
class TE_API GameplayUtils : public TFunctionLibrary
{
public:
    inline static const TEString StaticClassName = "GameplayUtils";

    // ── Build & Packaging Environment Queries ───────────────────────────
    static bool IsShippingBuild()
    {
#if defined(TE_SHIPPING)
        return true;
#else
        return false;
#endif
    }

    static bool IsDevelopmentBuild() { return !IsShippingBuild(); }

    static bool IsEditor()
    {
#if defined(TE_EDITOR)
        return true;
#else
        return false;
#endif
    }

    static bool IsStandalone() { return !IsEditor(); }

    // ── Container helpers ───────────────────────────────────────────────
    template <typename Container, typename Predicate>
    static auto FindFirst(Container &container, Predicate pred) -> decltype(container.Data())
    {
        for (size_t i = 0; i < container.Num(); ++i)
        {
            if (pred(container[i]))
                return &container[i];
        }
        return nullptr;
    }

    template <typename T> static bool Equals(const T &a, const T &b) { return a == b; }

    // ── Entity helpers ──────────────────────────────────────────────────
    static TEString GetEntityDisplayName(class EntityManager &mgr, Entity entity);
    static bool EntityContainsPoint(class EntityManager &mgr, Entity entity, const TEVector2 &worldPoint);
    static TEMatrix4 ResolveWorldTransform(class EntityManager &mgr, Entity entity, class TComponent *comp);
    static Entity PickEntity(Scene &scene, const TEVector2 &worldPoint);
    static TEVector2 ViewportPixelToWorld(TEVector2 pixelPos, TEVector2 viewportSize, TEVector2 cameraPos,
                                          float cameraZoom);
    static TEVector2 WorldToViewportPixel(TEVector2 worldPos, TEVector2 viewportSize, TEVector2 cameraPos,
                                          float cameraZoom);

    // ── Spawning, Lifecycle & Application Helpers ─────────────────────────
    static Entity SpawnEntity(Scene &scene, const TEString &name = "SpawnedEntity",
                              const TEVector2 &position = {0.0f, 0.0f});

    template <typename T, typename... Args>
    static Entity Spawn(Scene &scene, const TEString &name = "SpawnedEntity", const TEVector2 &position = {0.0f, 0.0f},
                        Args &&...args)
    {
        Entity entity = SpawnEntity(scene, name, position);
        if constexpr (!std::is_same_v<T, void>)
        {
            entity.template AddComponent<T>(std::forward<Args>(args)...);
        }
        return entity;
    }

    static void Destroy(Scene &scene, Entity entity);
    static void QuitGame();

    static Entity SpawnControllableGameObject(Scene &scene, const TEString &name = "ControllableObject",
                                              const TEVector2 &position = {0.0f, 0.0f});
    static TERef<class UIWidget> CreateWidget(const TEString &uiAssetPath);

    // ── Spatial Sweep Pipeline ───────────────────────────────────────────
    static TESpatialHitResult SweepLine(const TEVector2 &start, const TEVector2 &end, Scene *scene = nullptr,
                                        bool drawDebug = false, float debugDuration = 0.0f,
                                        const TEColor &traceColor = TEColor(0.0f, 1.0f, 0.0f, 1.0f),
                                        const TEColor &hitColor = TEColor(1.0f, 0.0f, 0.0f, 1.0f));
    static TEArray<TESpatialHitResult> MultiSweepLine(const TEVector2 &start, const TEVector2 &end,
                                                      Scene *scene = nullptr, bool drawDebug = false,
                                                      float debugDuration = 0.0f,
                                                      const TEColor &traceColor = TEColor(0.0f, 1.0f, 0.0f, 1.0f),
                                                      const TEColor &hitColor = TEColor(1.0f, 0.0f, 0.0f, 1.0f));

    static TESpatialHitResult SweepCircle(const TEVector2 &start, const TEVector2 &end,
                                          const TEVector2 &radius = {1.0f, 1.0f}, float angleRadians = 0.0f,
                                          Scene *scene = nullptr, bool drawDebug = false, float debugDuration = 0.0f,
                                          const TEColor &traceColor = TEColor(0.0f, 1.0f, 0.0f, 1.0f),
                                          const TEColor &hitColor = TEColor(1.0f, 0.0f, 0.0f, 1.0f));
    static TEArray<TESpatialHitResult> MultiSweepCircle(const TEVector2 &start, const TEVector2 &end,
                                                        const TEVector2 &radius = {1.0f, 1.0f},
                                                        float angleRadians = 0.0f, Scene *scene = nullptr,
                                                        bool drawDebug = false, float debugDuration = 0.0f,
                                                        const TEColor &traceColor = TEColor(0.0f, 1.0f, 0.0f, 1.0f),
                                                        const TEColor &hitColor = TEColor(1.0f, 0.0f, 0.0f, 1.0f));

    static TESpatialHitResult SweepBox(const TEVector2 &start, const TEVector2 &end, const TEVector2 &halfExtents,
                                       float angleRadians = 0.0f, Scene *scene = nullptr, bool drawDebug = false,
                                       float debugDuration = 0.0f,
                                       const TEColor &traceColor = TEColor(0.0f, 1.0f, 0.0f, 1.0f),
                                       const TEColor &hitColor = TEColor(1.0f, 0.0f, 0.0f, 1.0f));
    static TEArray<TESpatialHitResult> MultiSweepBox(const TEVector2 &start, const TEVector2 &end,
                                                     const TEVector2 &halfExtents, float angleRadians = 0.0f,
                                                     Scene *scene = nullptr, bool drawDebug = false,
                                                     float debugDuration = 0.0f,
                                                     const TEColor &traceColor = TEColor(0.0f, 1.0f, 0.0f, 1.0f),
                                                     const TEColor &hitColor = TEColor(1.0f, 0.0f, 0.0f, 1.0f));

    static TESpatialHitResult SweepCustom(const TEVector2 &start, const TEVector2 &end,
                                          const struct CollisionShape &shape, float angleRadians = 0.0f,
                                          Scene *scene = nullptr, bool drawDebug = false, float debugDuration = 0.0f,
                                          const TEColor &traceColor = TEColor(0.0f, 1.0f, 0.0f, 1.0f),
                                          const TEColor &hitColor = TEColor(1.0f, 0.0f, 0.0f, 1.0f));
    static TEArray<TESpatialHitResult> MultiSweepCustom(const TEVector2 &start, const TEVector2 &end,
                                                        const struct CollisionShape &shape, float angleRadians = 0.0f,
                                                        Scene *scene = nullptr, bool drawDebug = false,
                                                        float debugDuration = 0.0f,
                                                        const TEColor &traceColor = TEColor(0.0f, 1.0f, 0.0f, 1.0f),
                                                        const TEColor &hitColor = TEColor(1.0f, 0.0f, 0.0f, 1.0f));
};

// Aliases for convenience
class TFSpatialQuery : public GameplayUtils
{
};

#define TE_PLUGIN_ENABLED(PluginName)                                                                                  \
    (defined(TE_PLUGIN_##PluginName##_ENABLED) && (TE_PLUGIN_##PluginName##_ENABLED == 1))
#define TE_IS_PLUGIN_ENABLED(PluginName)                                                                               \
    (defined(TE_PLUGIN_##PluginName##_ENABLED) && (TE_PLUGIN_##PluginName##_ENABLED == 1))
