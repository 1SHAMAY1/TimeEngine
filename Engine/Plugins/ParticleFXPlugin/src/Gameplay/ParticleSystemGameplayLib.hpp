#pragma once
#include "Core/PreRequisites.h"
#include "GameFrameWork/TFunctionLibrary.hpp"
#include "Core/Scene/EntityManager.hpp"
#include "Renderer/TEColor.hpp"
#include "Utils/MathUtils.hpp"
#include "../Components/ParticleEmitterComponent.hpp"


class ParticleSystemGameplayLib : public TFunctionLibrary
{
public:
    inline static const TEString StaticClassName = "ParticleSystemGameplayLib";

    // ── Spawning & Emitter Lifecycle ────────────────────────────────────
    // Spawns a particle emitter entity at world position with specified preset ("Fire", "Sparks", "Explosion", "Smoke", "Rain")
    static Entity SpawnEmitterAtLocation(EntityManager &em, const TEVector &location,
                                         const TEString &preset = "Fire",
                                         float autoDestroyDuration = -1.0f);

    // Spawns an emitter attached to a parent entity
    static Entity SpawnEmitterAttached(EntityManager &em, Entity parentEntity,
                                       const TEVector &relativeOffset = TEVector(0.0f, 0.0f, 0.0f),
                                       const TEString &preset = "Sparks");

    // ── Combat & Hit Impact FX ──────────────────────────────────────────
    // Spawns a spark burst reflecting away from a surface normal
    static Entity SpawnHitImpactSparks(EntityManager &em, const TEVector &hitPoint,
                                       const TEVector &hitNormal,
                                       uint32_t sparkCount = 30,
                                       const TEColor &color = TEColor(1.0f, 0.95f, 0.5f, 1.0f),
                                       float bounciness = 0.65f);

    // Spawns an instantaneous radial explosion shockwave
    static Entity SpawnExplosion(EntityManager &em, const TEVector &location,
                                 uint32_t burstCount = 100,
                                 float speed = 8.0f,
                                 const TEColor &coreColor = TEColor(1.0f, 0.6f, 0.1f, 1.0f));

    // Spawns a rising smoke puff (muzzle flash, dust puff, landing impact)
    static Entity SpawnSmokePuff(EntityManager &em, const TEVector &location,
                                 float size = 0.8f, float lifetime = 2.0f,
                                 const TEColor &smokeColor = TEColor(0.6f, 0.6f, 0.6f, 0.5f));

    // ── Environmental & Weather FX ──────────────────────────────────────
    // Spawns rain/weather emitter over a specified width
    static Entity SpawnRainWeather(EntityManager &em, const TEVector &topCenter,
                                   float width = 12.0f, float rate = 160.0f);

    // ── Control & Trigger Actions ───────────────────────────────────────
    // Triggers an immediate burst on an existing emitter entity
    static bool TriggerBurst(EntityManager &em, Entity emitterEntity, uint32_t count = 30);

    // Sets playing state on an emitter entity
    static bool SetEmitterActive(EntityManager &em, Entity emitterEntity, bool active);

    // Cleans up / resets an emitter
    static bool ResetEmitter(EntityManager &em, Entity emitterEntity);

    // Destroys an emitter entity
    static bool StopAndDestroyEmitter(EntityManager &em, Entity emitterEntity);
};

