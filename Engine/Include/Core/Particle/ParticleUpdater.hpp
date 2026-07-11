#pragma once
#include "Core/Physics/PhysicsWorld.hpp"
#include "ParticlePool.hpp"

namespace TE
{

class ParticleUpdater
{
public:
    void Update(ParticlePool &pool, float deltaTime, PhysicsWorld *physicsWorld = nullptr,
                bool physicsSimulated = false, float bounciness = 0.5f)
    {
        for (auto &p : pool.Particles)
        {
            if (!p.Active)
                continue;

            if (physicsSimulated && physicsWorld)
            {
                TEVector2 start = {p.Position.x, p.Position.y};
                TEVector2 velocity2D = {p.Velocity.x, p.Velocity.y};
                float length = velocity2D.Length();

                if (length > 0.0001f)
                {
                    TEVector2 dir = velocity2D.Normalized();
                    float maxDistance = length * deltaTime;
                    TEVector2 hitPoint;
                    TEVector2 hitNormal;
                    float fraction = 0.0f;
                    uint32_t hitEntityID = 0;

                    if (physicsWorld->Raycast(start, dir, maxDistance, hitPoint, hitNormal, fraction, hitEntityID))
                    {
                        // Bounce particle position slightly away from the hit surface
                        TEVector2 bouncePos = hitPoint + hitNormal * 0.01f;
                        p.Position.x = bouncePos.x;
                        p.Position.y = bouncePos.y;

                        // Reflect velocity
                        float dotVal = Dot(velocity2D, hitNormal);
                        TEVector2 reflected = velocity2D - hitNormal * (2.0f * dotVal);
                        reflected = reflected * bounciness;

                        p.Velocity.x = reflected.x;
                        p.Velocity.y = reflected.y;
                    }
                    else
                    {
                        p.Position.x += p.Velocity.x * deltaTime;
                        p.Position.y += p.Velocity.y * deltaTime;
                        p.Position.z += p.Velocity.z * deltaTime;
                    }
                }
                else
                {
                    p.Position.x += p.Velocity.x * deltaTime;
                    p.Position.y += p.Velocity.y * deltaTime;
                    p.Position.z += p.Velocity.z * deltaTime;
                }
            }
            else
            {
                p.Position.x += p.Velocity.x * deltaTime;
                p.Position.y += p.Velocity.y * deltaTime;
                p.Position.z += p.Velocity.z * deltaTime;
            }

            p.Velocity.x += p.Acceleration.x * deltaTime;
            p.Velocity.y += p.Acceleration.y * deltaTime;
            p.Velocity.z += p.Acceleration.z * deltaTime;

            p.Lifetime -= deltaTime;
            p.Color.w = p.Lifetime / p.MaxLifetime; // fade alpha

            if (p.Lifetime <= 0.0f)
                p.Active = false;
        }
    }
};

} // namespace TE
