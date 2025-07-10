#pragma once
#include "ParticleEmitterComponent.hpp"

namespace TE {

    class ParticleSpawner {
    public:
        void Emit(ParticleEmitterComponent& emitter, float deltaTime) {
            emitter.Accumulator += emitter.EmitRate * deltaTime;

            while (emitter.Accumulator >= 1.0f) {
                Particle* p = emitter.Pool->Allocate();
                if (!p) return;

                p->Position = emitter.EmissionPosition;
                p->Velocity = emitter.BaseVelocity;
                p->Acceleration = emitter.BaseAcceleration;
                p->Color = emitter.StartColor;
                p->Size = emitter.ParticleSize;
                p->Lifetime = p->MaxLifetime = emitter.ParticleLife;
                p->Rotation = 0.0f;
                p->Active = true;

                emitter.Accumulator -= 1.0f;
            }
        }
    };

}
