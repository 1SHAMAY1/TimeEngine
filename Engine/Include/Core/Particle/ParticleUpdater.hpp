#pragma once
#include "ParticlePool.hpp"

namespace TE {

    class ParticleUpdater {
    public:
        void Update(ParticlePool& pool, float deltaTime) {
            for (auto& p : pool.Particles) {
                if (!p.Active) continue;

                p.Velocity += p.Acceleration * deltaTime;
                p.Position += p.Velocity * deltaTime;
                p.Lifetime -= deltaTime;

                p.Color.w = p.Lifetime / p.MaxLifetime; // fade alpha

                if (p.Lifetime <= 0.0f)
                    p.Active = false;
            }
        }
    };

}
