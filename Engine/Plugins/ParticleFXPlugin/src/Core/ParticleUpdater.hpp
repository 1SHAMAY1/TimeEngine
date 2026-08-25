#pragma once
#include "ParticlePool.hpp"
#include "Modules/ParticleModule.hpp"
#include "Modules/UpdateModules.hpp"

class ParticleUpdater
{
public:
    void AddModule(TERef<ParticleModule> module)
    {
        if (module)
            m_Modules.push_back(module);
    }

    void ClearModules()
    {
        m_Modules.clear();
    }

    void Update(EmitterContext &ctx, ParticlePool &pool, float dt)
    {
        for (auto &p : pool.GetParticles())
        {
            if (!p.Active)
                continue;

            // 1. Run all active update modules (Forces, Drag, Color, Size, Physics Raycast Collision)
            for (auto &mod : m_Modules)
            {
                if (mod->IsEnabled())
                    mod->OnParticleUpdate(ctx, p, dt);
            }

            // 2. Motion integration (Euler integration)
            p.Velocity += p.Acceleration * dt;
            p.Position += p.Velocity * dt;

            // 3. Rotation integration
            p.AngularVelocity = p.AngularVelocity;
            p.Rotation += p.AngularVelocity * dt;

            // 4. Age particle
            p.Lifetime -= dt;

            // 5. Deactivate expired particle
            if (p.Lifetime <= 0.0f)
            {
                pool.Free(&p);
            }
        }
    }

    const TEArray<TERef<ParticleModule>> &GetModules() const { return m_Modules; }
    TEArray<TERef<ParticleModule>> &GetModules() { return m_Modules; }

private:
    TEArray<TERef<ParticleModule>> m_Modules;
};

