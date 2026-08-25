#pragma once
#include "ParticlePool.hpp"
#include "Modules/ParticleModule.hpp"
#include "Modules/SpawnModules.hpp"



class ParticleSpawner
{
public:
    void AddModule(TERef<ParticleModule> module)
    {
        if (module)
            m_Modules.Add(module);
    }

    void ClearModules()
    {
        m_Modules.clear();
    }

    void Spawn(EmitterContext &ctx, ParticlePool &pool, float dt)
    {
        size_t initialActiveCount = pool.GetActiveCount();

        // 1. Run Emitter Spawn modules (allocates new particles)
        for (auto &mod : m_Modules)
        {
            if (mod->IsEnabled())
                mod->OnEmitterSpawn(ctx, pool, dt);
        }

        // 2. For all newly spawned particles, run Particle Spawn modules
        for (auto &p : pool.GetParticles())
        {
            if (p.Active && p.Lifetime <= 0.0f) // Newly allocated uninitialized particle
            {
                // Set default lifetime
                p.Lifetime = 1.0f;
                p.MaxLifetime = 1.0f;
                p.Size = 1.0f;
                p.BaseSize = 1.0f;
                p.Position = ctx.SimSpace == ESimulationSpace::World ? ctx.WorldPosition : TEVector{0.0f, 0.0f, 0.0f};
                p.Velocity = {0.0f, 0.0f, 0.0f};
                p.Acceleration = {0.0f, 0.0f, 0.0f};
                p.Color = {1.0f, 1.0f, 1.0f, 1.0f};
                p.BaseColor = {1.0f, 1.0f, 1.0f, 1.0f};

                for (auto &mod : m_Modules)
                {
                    if (mod->IsEnabled())
                        mod->OnParticleSpawn(ctx, p);
                }
            }
        }
    }

    const TEArray<TERef<ParticleModule>> &GetModules() const { return m_Modules; }
    TEArray<TERef<ParticleModule>> &GetModules() { return m_Modules; }

private:
    TEArray<TERef<ParticleModule>> m_Modules;
};

