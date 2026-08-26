#pragma once
#include "../Core/Modules/SpawnModules.hpp"
#include "../Core/Modules/UpdateModules.hpp"
#include "../Core/ParticlePool.hpp"
#include "../Core/ParticleSpawner.hpp"
#include "../Core/ParticleTypes.hpp"
#include "../Core/ParticleUpdater.hpp"
#include "Core/PreRequisites.h"
#include "Core/Scene/ComponentRegistry.hpp"
#include "GameFrameWork/TComponent.hpp"
#include "Renderer/TEColor.hpp"
#include "Utils/MathUtils.hpp"
#include <memory>

class ParticleEmitterComponent : public TComponent
{
public:
    GENERATED_BODY(ParticleEmitterComponent)

    // === Emitter Settings ===
    T_PROPERTY(bool, Playing, "Playing", true)
    T_PROPERTY(bool, Loop, "Loop", true)
    T_PROPERTY(float, MaxCapacity, "Max Capacity", 2000.0f)

    // === Emitter Spawn ===
    T_PROPERTY(bool, EnableRate, "Enable Rate", true)
    T_PROPERTY(float, EmitRate, "Emit Rate", 60.0f)
    T_PROPERTY(bool, EnableBurst, "Enable Burst", false)
    T_PROPERTY(float, BurstCount, "Burst Count", 25.0f)
    T_PROPERTY(float, BurstInterval, "Burst Interval", 1.0f)

    // === Particle Spawn ===
    T_PROPERTY(float, ShapeRadius, "Shape Radius", 0.5f)
    T_PROPERTY(float, ShapeArcAngle, "Shape Arc Angle", 45.0f)
    T_PROPERTY(TEVector, ShapeBoxExtents, "Shape Box Extents", TEVector(1.0f, 1.0f, 0.0f))

    T_PROPERTY(TEVector, BaseDirection, "Direction", TEVector(0.0f, 1.0f, 0.0f))
    T_PROPERTY(float, SpeedMin, "Speed Min", 2.0f)
    T_PROPERTY(float, SpeedMax, "Speed Max", 5.0f)
    T_PROPERTY(float, SpreadAngle, "Spread Angle", 30.0f)

    T_PROPERTY(float, LifetimeMin, "Lifetime Min", 1.0f)
    T_PROPERTY(float, LifetimeMax, "Lifetime Max", 2.5f)

    T_PROPERTY(float, SizeMin, "Size Min", 0.4f)
    T_PROPERTY(float, SizeMax, "Size Max", 0.8f)
    T_PROPERTY(TEColor, StartColor, "Start Color", TEColor::White())
    T_PROPERTY(TEColor, EndColor, "End Color", TEColor(1.0f, 1.0f, 1.0f, 0.0f))

    // === Particle Update ===
    T_PROPERTY(TEVector, Gravity, "Gravity", TEVector(0.0f, -9.81f, 0.0f))
    T_PROPERTY(float, LinearDrag, "Linear Drag", 0.1f)

    T_PROPERTY(bool, PhysicsSimulated, "Physics Collision", false)
    T_PROPERTY(float, Bounciness, "Bounciness", 0.6f)
    T_PROPERTY(float, Friction, "Friction", 0.1f)

    EEmissionShape EmissionShape = EEmissionShape::Circle2D;
    ESimulationSpace SimSpace = ESimulationSpace::World;
    EParticleBlendMode BlendMode = EParticleBlendMode::AlphaBlend;

    ParticleEmitterComponent();
    virtual ~ParticleEmitterComponent() override = default;

    virtual TEString GetClassName() const override { return StaticClassName; }

    void SyncModules();
    void Update(float dt, const TEVector &worldPos, class PhysicsWorld *physicsWorld = nullptr);
    void Reset();

    ParticlePool &GetPool() { return m_Pool; }
    const ParticlePool &GetPool() const { return m_Pool; }

    // Preset configurations
    void ApplyFirePreset();
    void ApplySparksPreset();
    void ApplyExplosionPreset();
    void ApplySmokePreset();
    void ApplyRainPreset();

private:
    ParticlePool m_Pool;
    ParticleSpawner m_Spawner;
    ParticleUpdater m_Updater;

    TERef<SpawnRateModule> m_RateModule;
    TERef<BurstSpawnModule> m_BurstModule;
    TERef<ShapeLocationModule> m_ShapeModule;
    TERef<InitialVelocityModule> m_VelocityModule;
    TERef<InitialLifetimeModule> m_LifetimeModule;
    TERef<InitialSizeColorModule> m_SizeColorModule;

    TERef<GravityForceModule> m_GravityModule;
    TERef<LinearDragModule> m_DragModule;
    TERef<ColorOverLifeModule> m_ColorModule;
    TERef<SizeOverLifeModule> m_SizeModule;
    TERef<PhysicsCollisionModule> m_CollisionModule;

    bool m_ModulesInitialized = false;
};

#ifdef TE_EDITOR
T_ENUM(EEmissionShape, {"Point", EEmissionShape::Point}, {"Circle 2D", EEmissionShape::Circle2D},
       {"Box 2D", EEmissionShape::Box2D}, {"Cone 2D", EEmissionShape::Cone2D}, {"Sphere 3D", EEmissionShape::Sphere3D},
       {"Box 3D", EEmissionShape::Box3D})

T_ENUM(ESimulationSpace, {"World", ESimulationSpace::World}, {"Local", ESimulationSpace::Local})

T_ENUM(EParticleBlendMode, {"Alpha Blend", EParticleBlendMode::AlphaBlend}, {"Additive", EParticleBlendMode::Additive},
       {"Multiply", EParticleBlendMode::Multiply}, {"Opaque", EParticleBlendMode::Opaque})

T_REGISTER_COMPONENT(ParticleEmitterComponent, "Particle Emitter")
T_REGISTER_PROPERTY(ParticleEmitterComponent, bool, Playing, "Playing")
T_REGISTER_PROPERTY(ParticleEmitterComponent, bool, Loop, "Loop")
T_REGISTER_PROPERTY(ParticleEmitterComponent, float, MaxCapacity, "Max Capacity")
T_REGISTER_ENUM_PROPERTY(ParticleEmitterComponent, ESimulationSpace, SimSpace, "Simulation Space")
T_REGISTER_ENUM_PROPERTY(ParticleEmitterComponent, EParticleBlendMode, BlendMode, "Blend Mode")

// Emitter Spawn
T_REGISTER_PROPERTY(ParticleEmitterComponent, bool, EnableRate, "Enable Rate")
T_REGISTER_PROPERTY(ParticleEmitterComponent, float, EmitRate, "Emit Rate")
T_REGISTER_PROPERTY(ParticleEmitterComponent, bool, EnableBurst, "Enable Burst")
T_REGISTER_PROPERTY(ParticleEmitterComponent, float, BurstCount, "Burst Count")
T_REGISTER_PROPERTY(ParticleEmitterComponent, float, BurstInterval, "Burst Interval")

// Particle Spawn
T_REGISTER_ENUM_PROPERTY(ParticleEmitterComponent, EEmissionShape, EmissionShape, "Emission Shape")
T_REGISTER_PROPERTY(ParticleEmitterComponent, float, ShapeRadius, "Shape Radius")
T_REGISTER_PROPERTY(ParticleEmitterComponent, float, ShapeArcAngle, "Shape Arc Angle")
T_REGISTER_PROPERTY(ParticleEmitterComponent, TEVector, ShapeBoxExtents, "Shape Box Extents")

T_REGISTER_PROPERTY(ParticleEmitterComponent, TEVector, BaseDirection, "Direction")
T_REGISTER_PROPERTY(ParticleEmitterComponent, float, SpeedMin, "Speed Min")
T_REGISTER_PROPERTY(ParticleEmitterComponent, float, SpeedMax, "Speed Max")
T_REGISTER_PROPERTY(ParticleEmitterComponent, float, SpreadAngle, "Spread Angle")

T_REGISTER_PROPERTY(ParticleEmitterComponent, float, LifetimeMin, "Lifetime Min")
T_REGISTER_PROPERTY(ParticleEmitterComponent, float, LifetimeMax, "Lifetime Max")

T_REGISTER_PROPERTY(ParticleEmitterComponent, float, SizeMin, "Size Min")
T_REGISTER_PROPERTY(ParticleEmitterComponent, float, SizeMax, "Size Max")
T_REGISTER_PROPERTY(ParticleEmitterComponent, TEColor, StartColor, "Start Color")
T_REGISTER_PROPERTY(ParticleEmitterComponent, TEColor, EndColor, "End Color")

// Particle Update
T_REGISTER_PROPERTY(ParticleEmitterComponent, TEVector, Gravity, "Gravity")
T_REGISTER_PROPERTY(ParticleEmitterComponent, float, LinearDrag, "Linear Drag")
T_REGISTER_PROPERTY(ParticleEmitterComponent, bool, PhysicsSimulated, "Physics Collision")
T_REGISTER_PROPERTY(ParticleEmitterComponent, float, Bounciness, "Bounciness")
T_REGISTER_PROPERTY(ParticleEmitterComponent, float, Friction, "Friction")

#endif
