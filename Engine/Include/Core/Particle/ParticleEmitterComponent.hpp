#pragma once
#include "Utility/MathUtils.hpp"
#include "ParticlePool.hpp"

namespace TE {

    struct ParticleEmitterComponent {
        TEVector EmissionPosition;
        TEVector BaseVelocity = { 0.0f, 1.0f, 0.0f };
        TEVector BaseAcceleration = { 0.0f, -9.8f, 0.0f };
        TEVector4 StartColor = { 1.0f, 1.0f, 1.0f, 1.0f };

        float EmitRate = 50.0f; // particles/sec
        float ParticleLife = 2.0f;
        float ParticleSize = 1.0f;

        float Accumulator = 0.0f;
        ParticlePool* Pool = nullptr;
    };

}
