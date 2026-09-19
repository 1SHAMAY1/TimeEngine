#pragma once
#include "ParticleModule.hpp"
#include <cmath>
#include <random>

inline float RandomFloat(float minVal, float maxVal)
{
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(minVal, maxVal);
    return dis(gen);
}

class SpawnRateModule : public ParticleModule
{
public:
    TEString GetName() const override { return "Spawn Rate"; }
    TEString GetCategory() const override { return "Emitter Spawn"; }

    float Rate = 60.0f; // particles per second

    void OnEmitterSpawn(EmitterContext &ctx, ParticlePool &pool, float dt) override
    {
        if (!m_Enabled || Rate <= 0.0f)
            return;

        m_Accumulator += Rate * dt;
        while (m_Accumulator >= 1.0f)
        {
            Particle *p = pool.Allocate();
            if (!p)
                break;

            p->Active = true;
            m_Accumulator -= 1.0f;
        }
    }

private:
    float m_Accumulator = 0.0f;
};

class BurstSpawnModule : public ParticleModule
{
public:
    TEString GetName() const override { return "Burst Instantaneous"; }
    TEString GetCategory() const override { return "Emitter Spawn"; }

    BurstConfig Config;

    void OnEmitterSpawn(EmitterContext &ctx, ParticlePool &pool, float dt) override
    {
        if (!m_Enabled || !Config.Enabled || Config.Count == 0)
            return;

        if (Config.Cycles > 0 && Config.CompletedCycles >= Config.Cycles)
            return;

        Config.Timer += dt;
        if (Config.Timer >= (Config.CompletedCycles == 0 ? Config.Delay : Config.Interval))
        {
            for (uint32_t i = 0; i < Config.Count; ++i)
            {
                Particle *p = pool.Allocate();
                if (!p)
                    break;
                p->Active = true;
            }
            Config.Timer = 0.0f;
            Config.CompletedCycles++;
        }
    }

    void Reset()
    {
        Config.CompletedCycles = 0;
        Config.Timer = 0.0f;
    }
};

class ShapeLocationModule : public ParticleModule
{
public:
    TEString GetName() const override { return "Shape Location"; }
    TEString GetCategory() const override { return "Particle Spawn"; }

    EEmissionShape Shape = EEmissionShape::Circle2D;
    float Radius = 0.5f;
    TEVector BoxExtents = {1.0f, 1.0f, 0.0f};
    float ArcAngle = 45.0f; // in degrees for Cone2D
    bool SurfaceOnly = false;

    void OnParticleSpawn(EmitterContext &ctx, Particle &particle) override
    {
        if (!m_Enabled)
            return;

        TEVector offset = {0.0f, 0.0f, 0.0f};

        switch (Shape)
        {
        case EEmissionShape::Point:
            offset = {0.0f, 0.0f, 0.0f};
            break;

        case EEmissionShape::Circle2D:
        {
            float angle = RandomFloat(0.0f, 6.2831853f);
            float r = SurfaceOnly ? Radius : Radius * std::sqrt(RandomFloat(0.0f, 1.0f));
            offset.x = std::cos(angle) * r;
            offset.y = std::sin(angle) * r;
            offset.z = 0.0f;
            break;
        }

        case EEmissionShape::Box2D:
        {
            offset.x = RandomFloat(-BoxExtents.x * 0.5f, BoxExtents.x * 0.5f);
            offset.y = RandomFloat(-BoxExtents.y * 0.5f, BoxExtents.y * 0.5f);
            offset.z = 0.0f;
            break;
        }

        case EEmissionShape::Cone2D:
        {
            float halfArc = (ArcAngle * 0.5f) * (3.14159265f / 180.0f);
            float angle = RandomFloat(-halfArc, halfArc) + 1.5707963f; // Upwards bias
            float r = SurfaceOnly ? Radius : Radius * RandomFloat(0.0f, 1.0f);
            offset.x = std::cos(angle) * r;
            offset.y = std::sin(angle) * r;
            offset.z = 0.0f;
            break;
        }

        case EEmissionShape::Sphere3D:
        {
            float u = RandomFloat(0.0f, 1.0f);
            float v = RandomFloat(0.0f, 1.0f);
            float theta = u * 2.0f * 3.14159265f;
            float phi = std::acos(2.0f * v - 1.0f);
            float r = SurfaceOnly ? Radius : Radius * std::cbrt(RandomFloat(0.0f, 1.0f));
            offset.x = r * std::sin(phi) * std::cos(theta);
            offset.y = r * std::sin(phi) * std::sin(theta);
            offset.z = r * std::cos(phi);
            break;
        }

        case EEmissionShape::Box3D:
        {
            offset.x = RandomFloat(-BoxExtents.x * 0.5f, BoxExtents.x * 0.5f);
            offset.y = RandomFloat(-BoxExtents.y * 0.5f, BoxExtents.y * 0.5f);
            offset.z = RandomFloat(-BoxExtents.z * 0.5f, BoxExtents.z * 0.5f);
            break;
        }
        }

        if (ctx.SimSpace == ESimulationSpace::World)
            particle.Position = ctx.WorldPosition + offset;
        else
            particle.Position = offset;
    }
};

class InitialVelocityModule : public ParticleModule
{
public:
    TEString GetName() const override { return "Add Velocity"; }
    TEString GetCategory() const override { return "Particle Spawn"; }

    TEVector Direction = {0.0f, 1.0f, 0.0f};
    float SpeedMin = 2.0f;
    float SpeedMax = 5.0f;
    float SpreadAngle = 30.0f; // degrees

    void OnParticleSpawn(EmitterContext &ctx, Particle &particle) override
    {
        if (!m_Enabled)
            return;

        float speed = RandomFloat(SpeedMin, SpeedMax);
        float spreadRad = (SpreadAngle * 0.5f) * (3.14159265f / 180.0f);
        float angleOffset = RandomFloat(-spreadRad, spreadRad);

        float baseAngle = std::atan2(Direction.y, Direction.x);
        float finalAngle = baseAngle + angleOffset;

        particle.Velocity.x = std::cos(finalAngle) * speed;
        particle.Velocity.y = std::sin(finalAngle) * speed;
        particle.Velocity.z = Direction.z * speed;
    }
};

class InitialLifetimeModule : public ParticleModule
{
public:
    TEString GetName() const override { return "Lifetime"; }
    TEString GetCategory() const override { return "Particle Spawn"; }

    float LifetimeMin = 1.0f;
    float LifetimeMax = 2.5f;

    void OnParticleSpawn(EmitterContext &ctx, Particle &particle) override
    {
        if (!m_Enabled)
            return;

        float life = RandomFloat(LifetimeMin, LifetimeMax);
        particle.Lifetime = life;
        particle.MaxLifetime = (life > 0.0001f) ? life : 0.0001f;
    }
};

class InitialSizeColorModule : public ParticleModule
{
public:
    TEString GetName() const override { return "Size & Color"; }
    TEString GetCategory() const override { return "Particle Spawn"; }

    float SizeMin = 0.5f;
    float SizeMax = 1.2f;
    TEVector4 StartColor = {1.0f, 1.0f, 1.0f, 1.0f};
    float RotationMin = 0.0f;
    float RotationMax = 360.0f;
    float AngularVelocityMin = -45.0f;
    float AngularVelocityMax = 45.0f;

    void OnParticleSpawn(EmitterContext &ctx, Particle &particle) override
    {
        if (!m_Enabled)
            return;

        float s = RandomFloat(SizeMin, SizeMax);
        particle.BaseSize = s;
        particle.Size = s;
        particle.BaseColor = StartColor;
        particle.Color = StartColor;
        particle.Rotation = RandomFloat(RotationMin, RotationMax) * (3.14159265f / 180.0f);
        particle.AngularVelocity = RandomFloat(AngularVelocityMin, AngularVelocityMax) * (3.14159265f / 180.0f);
    }
};
