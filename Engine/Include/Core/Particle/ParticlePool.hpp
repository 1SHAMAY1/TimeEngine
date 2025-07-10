#pragma once
#include <vector>
#include "ParticleTypes.hpp"

namespace TE {

    class ParticlePool {
    public:
        std::vector<Particle> Particles;
        size_t Capacity;

        ParticlePool(size_t maxCount = 1000) : Capacity(maxCount) {
            Particles.resize(Capacity);
        }

        Particle* Allocate() {
            for (auto& p : Particles) {
                if (!p.Active)
                    return &p;
            }
            return nullptr;
        }

        void Clear() {
            for (auto& p : Particles)
                p.Active = false;
        }
    };

}
