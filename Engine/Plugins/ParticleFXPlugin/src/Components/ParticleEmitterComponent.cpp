#include "ParticleEmitterComponent.hpp"
#include "Core/Scene/TransformComponent.hpp"


ParticleEmitterComponent::ParticleEmitterComponent()
    : m_Pool(2000)
{
    m_RateModule = CreateRef<SpawnRateModule>();
    m_BurstModule = CreateRef<BurstSpawnModule>();
    m_ShapeModule = CreateRef<ShapeLocationModule>();
    m_VelocityModule = CreateRef<InitialVelocityModule>();
    m_LifetimeModule = CreateRef<InitialLifetimeModule>();
    m_SizeColorModule = CreateRef<InitialSizeColorModule>();

    m_GravityModule = CreateRef<GravityForceModule>();
    m_DragModule = CreateRef<LinearDragModule>();
    m_ColorModule = CreateRef<ColorOverLifeModule>();
    m_SizeModule = CreateRef<SizeOverLifeModule>();
    m_CollisionModule = CreateRef<PhysicsCollisionModule>();

    // Register into Spawner stack
    m_Spawner.AddModule(m_RateModule);
    m_Spawner.AddModule(m_BurstModule);
    m_Spawner.AddModule(m_ShapeModule);
    m_Spawner.AddModule(m_VelocityModule);
    m_Spawner.AddModule(m_LifetimeModule);
    m_Spawner.AddModule(m_SizeColorModule);

    // Register into Updater stack
    m_Updater.AddModule(m_GravityModule);
    m_Updater.AddModule(m_DragModule);
    m_Updater.AddModule(m_ColorModule);
    m_Updater.AddModule(m_SizeModule);
    m_Updater.AddModule(m_CollisionModule);

    m_ModulesInitialized = true;
    SyncModules();
}

void ParticleEmitterComponent::SyncModules()
{
    if (!m_ModulesInitialized)
        return;

    if (m_Pool.GetCapacity() != (size_t)MaxCapacity && MaxCapacity > 0)
        m_Pool.Resize((size_t)MaxCapacity);

    // Emitter Spawn
    m_RateModule->SetEnabled(EnableRate);
    m_RateModule->Rate = EmitRate;

    m_BurstModule->SetEnabled(EnableBurst);
    m_BurstModule->Config.Count = (uint32_t)BurstCount;
    m_BurstModule->Config.Interval = BurstInterval;

    // Particle Spawn
    m_ShapeModule->Shape = EmissionShape;
    m_ShapeModule->Radius = ShapeRadius;
    m_ShapeModule->ArcAngle = ShapeArcAngle;
    m_ShapeModule->BoxExtents = ShapeBoxExtents;

    m_VelocityModule->Direction = BaseDirection;
    m_VelocityModule->SpeedMin = SpeedMin;
    m_VelocityModule->SpeedMax = SpeedMax;
    m_VelocityModule->SpreadAngle = SpreadAngle;

    m_LifetimeModule->LifetimeMin = LifetimeMin;
    m_LifetimeModule->LifetimeMax = LifetimeMax;

    m_SizeColorModule->SizeMin = SizeMin;
    m_SizeColorModule->SizeMax = SizeMax;
    m_SizeColorModule->StartColor = TEVector4(StartColor.r, StartColor.g, StartColor.b, StartColor.a);

    // Particle Update
    m_GravityModule->Gravity = Gravity;
    m_DragModule->Drag = LinearDrag;

    // Color gradient setup
    m_ColorModule->Gradient.Stops.Clear();
    m_ColorModule->Gradient.Stops.Add({0.0f, TEVector4(StartColor.r, StartColor.g, StartColor.b, StartColor.a)});
    m_ColorModule->Gradient.Stops.Add({1.0f, TEVector4(EndColor.r, EndColor.g, EndColor.b, EndColor.a)});

    m_CollisionModule->SetEnabled(PhysicsSimulated);
    m_CollisionModule->Bounciness = Bounciness;
    m_CollisionModule->Friction = Friction;
}

void ParticleEmitterComponent::Update(float dt, const TEVector &worldPos, PhysicsWorld *physicsWorld)
{
    if (!Playing)
        return;

    SyncModules();

    EmitterContext ctx;
    ctx.WorldPosition = worldPos;
    ctx.SimSpace = SimSpace;
    ctx.DeltaTime = dt;
    ctx.PhysWorld = physicsWorld;

    m_Spawner.Spawn(ctx, m_Pool, dt);
    m_Updater.Update(ctx, m_Pool, dt);
}

void ParticleEmitterComponent::Reset()
{
    m_Pool.Clear();
    if (m_BurstModule)
        m_BurstModule->Reset();
}

void ParticleEmitterComponent::ApplyFirePreset()
{
    EnableRate = true;
    EmitRate = 120.0f;
    EnableBurst = false;
    EmissionShape = EEmissionShape::Circle2D;
    ShapeRadius = 0.25f;
    BaseDirection = TEVector(0.0f, 1.0f, 0.0f);
    SpeedMin = 1.5f;
    SpeedMax = 4.0f;
    SpreadAngle = 25.0f;
    LifetimeMin = 0.5f;
    LifetimeMax = 1.2f;
    SizeMin = 0.3f;
    SizeMax = 0.7f;
    StartColor = TEColor(1.0f, 0.8f, 0.2f, 1.0f);
    EndColor = TEColor(0.9f, 0.1f, 0.0f, 0.0f);
    Gravity = TEVector(0.0f, 0.5f, 0.0f); // buoyant rise
    LinearDrag = 0.2f;
    BlendMode = EParticleBlendMode::Additive;
    PhysicsSimulated = false;
    SyncModules();
}

void ParticleEmitterComponent::ApplySparksPreset()
{
    EnableRate = false;
    EnableBurst = true;
    BurstCount = 50.0f;
    BurstInterval = 1.5f;
    EmissionShape = EEmissionShape::Point;
    BaseDirection = TEVector(0.0f, 1.0f, 0.0f);
    SpeedMin = 4.0f;
    SpeedMax = 9.0f;
    SpreadAngle = 80.0f;
    LifetimeMin = 0.4f;
    LifetimeMax = 1.0f;
    SizeMin = 0.1f;
    SizeMax = 0.25f;
    StartColor = TEColor(1.0f, 0.95f, 0.6f, 1.0f);
    EndColor = TEColor(1.0f, 0.3f, 0.0f, 0.0f);
    Gravity = TEVector(0.0f, -9.81f, 0.0f);
    LinearDrag = 0.05f;
    BlendMode = EParticleBlendMode::Additive;
    PhysicsSimulated = true;
    Bounciness = 0.7f;
    SyncModules();
}

void ParticleEmitterComponent::ApplyExplosionPreset()
{
    EnableRate = false;
    EnableBurst = true;
    BurstCount = 100.0f;
    BurstInterval = 2.0f;
    EmissionShape = EEmissionShape::Circle2D;
    ShapeRadius = 0.1f;
    BaseDirection = TEVector(0.0f, 1.0f, 0.0f);
    SpeedMin = 5.0f;
    SpeedMax = 12.0f;
    SpreadAngle = 360.0f;
    LifetimeMin = 0.6f;
    LifetimeMax = 1.4f;
    SizeMin = 0.3f;
    SizeMax = 0.9f;
    StartColor = TEColor(1.0f, 0.7f, 0.1f, 1.0f);
    EndColor = TEColor(0.3f, 0.3f, 0.3f, 0.0f);
    Gravity = TEVector(0.0f, -2.0f, 0.0f);
    LinearDrag = 0.4f;
    BlendMode = EParticleBlendMode::Additive;
    PhysicsSimulated = true;
    Bounciness = 0.4f;
    SyncModules();
}

void ParticleEmitterComponent::ApplySmokePreset()
{
    EnableRate = true;
    EmitRate = 35.0f;
    EnableBurst = false;
    EmissionShape = EEmissionShape::Circle2D;
    ShapeRadius = 0.3f;
    BaseDirection = TEVector(0.0f, 1.0f, 0.0f);
    SpeedMin = 0.5f;
    SpeedMax = 1.8f;
    SpreadAngle = 40.0f;
    LifetimeMin = 1.5f;
    LifetimeMax = 3.0f;
    SizeMin = 0.5f;
    SizeMax = 1.4f;
    StartColor = TEColor(0.7f, 0.7f, 0.7f, 0.6f);
    EndColor = TEColor(0.2f, 0.2f, 0.2f, 0.0f);
    Gravity = TEVector(0.1f, 0.3f, 0.0f);
    LinearDrag = 0.3f;
    BlendMode = EParticleBlendMode::AlphaBlend;
    PhysicsSimulated = false;
    SyncModules();
}

void ParticleEmitterComponent::ApplyRainPreset()
{
    EnableRate = true;
    EmitRate = 180.0f;
    EnableBurst = false;
    EmissionShape = EEmissionShape::Box2D;
    ShapeBoxExtents = TEVector(10.0f, 0.5f, 0.0f);
    BaseDirection = TEVector(0.0f, -1.0f, 0.0f);
    SpeedMin = 10.0f;
    SpeedMax = 15.0f;
    SpreadAngle = 5.0f;
    LifetimeMin = 1.0f;
    LifetimeMax = 1.8f;
    SizeMin = 0.08f;
    SizeMax = 0.15f;
    StartColor = TEColor(0.7f, 0.85f, 1.0f, 0.8f);
    EndColor = TEColor(0.6f, 0.8f, 1.0f, 0.2f);
    Gravity = TEVector(0.0f, -9.81f, 0.0f);
    LinearDrag = 0.0f;
    BlendMode = EParticleBlendMode::AlphaBlend;
    PhysicsSimulated = true;
    Bounciness = 0.2f;
    SyncModules();
}

#ifdef TE_EDITOR
// Register Presets in Entity Creation menu
T_REGISTER_PRESET(FireEmitter, "Fire Emitter", "Particles & FX", [](EntityID id, EntityManager *em) {
    auto *emitter = em->AddComponent<ParticleEmitterComponent>(id);
    if (emitter)
        emitter->ApplyFirePreset();
})

T_REGISTER_PRESET(SparksEmitter, "Sparks Emitter", "Particles & FX", [](EntityID id, EntityManager *em) {
    auto *emitter = em->AddComponent<ParticleEmitterComponent>(id);
    if (emitter)
        emitter->ApplySparksPreset();
})

T_REGISTER_PRESET(ExplosionEmitter, "Explosion Emitter", "Particles & FX", [](EntityID id, EntityManager *em) {
    auto *emitter = em->AddComponent<ParticleEmitterComponent>(id);
    if (emitter)
        emitter->ApplyExplosionPreset();
})

T_REGISTER_PRESET(SmokeEmitter, "Smoke Emitter", "Particles & FX", [](EntityID id, EntityManager *em) {
    auto *emitter = em->AddComponent<ParticleEmitterComponent>(id);
    if (emitter)
        emitter->ApplySmokePreset();
})

T_REGISTER_PRESET(RainEmitter, "Rain / Weather Emitter", "Particles & FX", [](EntityID id, EntityManager *em) {
    auto *emitter = em->AddComponent<ParticleEmitterComponent>(id);
    if (emitter)
        emitter->ApplyRainPreset();
})
#endif

