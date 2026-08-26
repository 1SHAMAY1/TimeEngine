#include "ParticleSystemGameplayLib.hpp"
#include "Core/Scene/TagComponent.hpp"
#include "Core/Scene/TransformComponent.hpp"

Entity ParticleSystemGameplayLib::SpawnEmitterAtLocation(EntityManager &em, const TEVector &location,
                                                         const TEString &preset, float autoDestroyDuration)
{
    Entity entity = em.CreateEntity();
    entity.AddComponent<TagComponent>("ParticleEmitter_" + preset);
    entity.AddComponent<TransformComponent>(location);

    auto *emitter = entity.AddComponent<ParticleEmitterComponent>();
    if (emitter)
    {
        if (preset == "Fire")
            emitter->ApplyFirePreset();
        else if (preset == "Sparks")
            emitter->ApplySparksPreset();
        else if (preset == "Explosion")
            emitter->ApplyExplosionPreset();
        else if (preset == "Smoke")
            emitter->ApplySmokePreset();
        else if (preset == "Rain")
            emitter->ApplyRainPreset();
        else
            emitter->ApplyFirePreset();

        emitter->SyncModules();
    }

    return entity;
}

Entity ParticleSystemGameplayLib::SpawnEmitterAttached(EntityManager &em, Entity parentEntity,
                                                       const TEVector &relativeOffset, const TEString &preset)
{
    Entity entity = em.CreateEntity();
    entity.AddComponent<TagComponent>("ParticleEmitter_Attached");

    auto *trans = entity.AddComponent<TransformComponent>(relativeOffset);
    if (trans && parentEntity.IsValid())
    {
        trans->Parent = parentEntity.GetID();
        auto *parentTrans = parentEntity.GetComponent<TransformComponent>();
        if (parentTrans)
            parentTrans->Children.push_back(entity.GetID());
    }

    auto *emitter = entity.AddComponent<ParticleEmitterComponent>();
    if (emitter)
    {
        emitter->SimSpace = ESimulationSpace::Local;
        if (preset == "Fire")
            emitter->ApplyFirePreset();
        else if (preset == "Sparks")
            emitter->ApplySparksPreset();
        else if (preset == "Explosion")
            emitter->ApplyExplosionPreset();
        else if (preset == "Smoke")
            emitter->ApplySmokePreset();
        else if (preset == "Rain")
            emitter->ApplyRainPreset();

        emitter->SimSpace = ESimulationSpace::Local; // Preserve local space
        emitter->SyncModules();
    }

    return entity;
}

Entity ParticleSystemGameplayLib::SpawnHitImpactSparks(EntityManager &em, const TEVector &hitPoint,
                                                       const TEVector &hitNormal, uint32_t sparkCount,
                                                       const TEColor &color, float bounciness)
{
    Entity entity = em.CreateEntity();
    entity.AddComponent<TagComponent>("HitImpact_Sparks");
    entity.AddComponent<TransformComponent>(hitPoint);

    auto *emitter = entity.AddComponent<ParticleEmitterComponent>();
    if (emitter)
    {
        emitter->ApplySparksPreset();
        emitter->BurstCount = (float)sparkCount;
        emitter->BaseDirection = hitNormal;
        emitter->StartColor = color;
        emitter->Bounciness = bounciness;
        emitter->PhysicsSimulated = true;
        emitter->Loop = false;
        emitter->SyncModules();
    }

    return entity;
}

Entity ParticleSystemGameplayLib::SpawnExplosion(EntityManager &em, const TEVector &location, uint32_t burstCount,
                                                 float speed, const TEColor &coreColor)
{
    Entity entity = em.CreateEntity();
    entity.AddComponent<TagComponent>("Explosion_FX");
    entity.AddComponent<TransformComponent>(location);

    auto *emitter = entity.AddComponent<ParticleEmitterComponent>();
    if (emitter)
    {
        emitter->ApplyExplosionPreset();
        emitter->BurstCount = (float)burstCount;
        emitter->SpeedMax = speed;
        emitter->StartColor = coreColor;
        emitter->Loop = false;
        emitter->SyncModules();
    }

    return entity;
}

Entity ParticleSystemGameplayLib::SpawnSmokePuff(EntityManager &em, const TEVector &location, float size,
                                                 float lifetime, const TEColor &smokeColor)
{
    Entity entity = em.CreateEntity();
    entity.AddComponent<TagComponent>("SmokePuff_FX");
    entity.AddComponent<TransformComponent>(location);

    auto *emitter = entity.AddComponent<ParticleEmitterComponent>();
    if (emitter)
    {
        emitter->ApplySmokePreset();
        emitter->SizeMin = size * 0.5f;
        emitter->SizeMax = size * 1.2f;
        emitter->LifetimeMin = lifetime * 0.7f;
        emitter->LifetimeMax = lifetime * 1.2f;
        emitter->StartColor = smokeColor;
        emitter->Loop = false;
        emitter->SyncModules();
    }

    return entity;
}

Entity ParticleSystemGameplayLib::SpawnRainWeather(EntityManager &em, const TEVector &topCenter, float width,
                                                   float rate)
{
    Entity entity = em.CreateEntity();
    entity.AddComponent<TagComponent>("Weather_Rain");
    entity.AddComponent<TransformComponent>(topCenter);

    auto *emitter = entity.AddComponent<ParticleEmitterComponent>();
    if (emitter)
    {
        emitter->ApplyRainPreset();
        emitter->ShapeBoxExtents = TEVector(width, 0.5f, 0.0f);
        emitter->EmitRate = rate;
        emitter->Loop = true;
        emitter->SyncModules();
    }

    return entity;
}

bool ParticleSystemGameplayLib::TriggerBurst(EntityManager &em, Entity emitterEntity, uint32_t count)
{
    if (!emitterEntity.IsValid())
        return false;

    auto *emitter = emitterEntity.GetComponent<ParticleEmitterComponent>();
    if (!emitter)
        return false;

    emitter->BurstCount = (float)count;
    emitter->EnableBurst = true;
    emitter->SyncModules();
    return true;
}

bool ParticleSystemGameplayLib::SetEmitterActive(EntityManager &em, Entity emitterEntity, bool active)
{
    if (!emitterEntity.IsValid())
        return false;

    auto *emitter = emitterEntity.GetComponent<ParticleEmitterComponent>();
    if (!emitter)
        return false;

    emitter->Playing = active;
    return true;
}

bool ParticleSystemGameplayLib::ResetEmitter(EntityManager &em, Entity emitterEntity)
{
    if (!emitterEntity.IsValid())
        return false;

    auto *emitter = emitterEntity.GetComponent<ParticleEmitterComponent>();
    if (!emitter)
        return false;

    emitter->Reset();
    return true;
}

bool ParticleSystemGameplayLib::StopAndDestroyEmitter(EntityManager &em, Entity emitterEntity)
{
    if (!emitterEntity.IsValid())
        return false;

    em.DestroyEntity(emitterEntity);
    return true;
}
