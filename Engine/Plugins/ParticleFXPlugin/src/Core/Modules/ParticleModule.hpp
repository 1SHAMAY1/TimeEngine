#pragma once
#include "../ParticleTypes.hpp"
#include "../ParticlePool.hpp"


struct EmitterContext
{
    TEVector WorldPosition = {0.0f, 0.0f, 0.0f};
    TEVector LocalPosition = {0.0f, 0.0f, 0.0f};
    TEVector Rotation = {0.0f, 0.0f, 0.0f};
    TEVector Scale = {1.0f, 1.0f, 1.0f};
    ESimulationSpace SimSpace = ESimulationSpace::World;
    float DeltaTime = 0.016f;
    float TotalTime = 0.0f;
    class PhysicsWorld *PhysWorld = nullptr;
};

class ParticleModule
{
public:
    virtual ~ParticleModule() = default;

    virtual TEString GetName() const = 0;
    virtual TEString GetCategory() const = 0; // "Emitter Spawn", "Particle Spawn", "Particle Update", "Particle Render"

    virtual void OnEmitterSpawn(EmitterContext &ctx, ParticlePool &pool, float dt) {}
    virtual void OnParticleSpawn(EmitterContext &ctx, Particle &particle) {}
    virtual void OnParticleUpdate(EmitterContext &ctx, Particle &particle, float dt) {}

    bool IsEnabled() const { return m_Enabled; }
    void SetEnabled(bool enabled) { m_Enabled = enabled; }

protected:
    bool m_Enabled = true;
};

