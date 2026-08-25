#pragma once
#include "ParticleModule.hpp"
#include "Core/Physics/PhysicsWorld.hpp"
#include <algorithm>


class GravityForceModule : public ParticleModule
{
public:
    TEString GetName() const override { return "Gravity Force"; }
    TEString GetCategory() const override { return "Particle Update"; }

    TEVector Gravity = {0.0f, -9.81f, 0.0f};

    void OnParticleUpdate(EmitterContext &ctx, Particle &particle, float dt) override
    {
        if (!m_Enabled)
            return;

        particle.Velocity += Gravity * dt;
    }
};

class LinearDragModule : public ParticleModule
{
public:
    TEString GetName() const override { return "Linear Drag"; }
    TEString GetCategory() const override { return "Particle Update"; }

    float Drag = 0.1f;

    void OnParticleUpdate(EmitterContext &ctx, Particle &particle, float dt) override
    {
        if (!m_Enabled || Drag <= 0.0f)
            return;

        float damping = std::max(0.0f, 1.0f - Drag * dt);
        particle.Velocity *= damping;
    }
};

class ColorOverLifeModule : public ParticleModule
{
public:
    TEString GetName() const override { return "Color Over Life"; }
    TEString GetCategory() const override { return "Particle Update"; }

    ColorGradient Gradient;

    void OnParticleUpdate(EmitterContext &ctx, Particle &particle, float dt) override
    {
        if (!m_Enabled)
            return;

        float normalizedAge = 1.0f - (particle.Lifetime / particle.MaxLifetime);
        TEVector4 gradColor = Gradient.Evaluate(normalizedAge);
        particle.Color = TEVector4(particle.BaseColor.x * gradColor.x,
                                   particle.BaseColor.y * gradColor.y,
                                   particle.BaseColor.z * gradColor.z,
                                   particle.BaseColor.w * gradColor.w);
    }
};

class SizeOverLifeModule : public ParticleModule
{
public:
    TEString GetName() const override { return "Size Over Life"; }
    TEString GetCategory() const override { return "Particle Update"; }

    FloatCurve Curve;

    void OnParticleUpdate(EmitterContext &ctx, Particle &particle, float dt) override
    {
        if (!m_Enabled)
            return;

        float normalizedAge = 1.0f - (particle.Lifetime / particle.MaxLifetime);
        float scale = Curve.Evaluate(normalizedAge);
        particle.Size = particle.BaseSize * scale;
    }
};

class PhysicsCollisionModule : public ParticleModule
{
public:
    TEString GetName() const override { return "Physics Collision"; }
    TEString GetCategory() const override { return "Particle Update"; }

    float Bounciness = 0.6f;
    float Friction = 0.1f;

    void OnParticleUpdate(EmitterContext &ctx, Particle &particle, float dt) override
    {
        if (!m_Enabled || !ctx.PhysWorld)
            return;

        TEVector2 start = {particle.Position.x, particle.Position.y};
        TEVector2 vel2D = {particle.Velocity.x, particle.Velocity.y};
        float speed = vel2D.Length();

        if (speed > 0.0001f)
        {
            TEVector2 dir = vel2D.Normalized();
            float maxDist = speed * dt;
            TEVector2 hitPoint;
            TEVector2 hitNormal;
            float fraction = 0.0f;
            uint32_t hitEntityID = 0;

            if (ctx.PhysWorld->Raycast(start, dir, maxDist, hitPoint, hitNormal, fraction, hitEntityID))
            {
                // Position adjustment just off collision surface
                TEVector2 bouncePos = hitPoint + hitNormal * 0.015f;
                particle.Position.x = bouncePos.x;
                particle.Position.y = bouncePos.y;

                // Reflect velocity
                float dotVal = Dot(vel2D, hitNormal);
                TEVector2 reflected = vel2D - hitNormal * (2.0f * dotVal);
                reflected = reflected * Bounciness;

                particle.Velocity.x = reflected.x * (1.0f - Friction);
                particle.Velocity.y = reflected.y * (1.0f - Friction);
            }
        }
    }
};

