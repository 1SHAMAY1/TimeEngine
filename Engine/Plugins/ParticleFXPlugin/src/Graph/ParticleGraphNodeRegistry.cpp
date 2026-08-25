#include "Graph/ParticleGraphNodeRegistry.hpp"
#include "Graph/ParticleGraphNodes.hpp"


void ParticleGraphNodeRegistry::RegisterAllNodes(NodePalettePopup &palette)
{
    palette.Clear();

    palette.RegisterNode("Particle Emitter Root", "Emitter", "ParticleEmitterRoot",
                         "Master particle emitter configuration and module inputs",
                         []() { return CreateRef<ParticleEmitterRootNode>(); });

    palette.RegisterNode("Spawn Rate", "Spawn", "ParticleSpawnRate",
                         "Controls constant and burst particle spawning rates",
                         []() { return CreateRef<ParticleSpawnRateNode>(); });

    palette.RegisterNode("Spawn Location", "Spawn", "ParticleSpawnLocation",
                         "Spawns particles across box, sphere, circle or point volumes",
                         []() { return CreateRef<ParticleSpawnLocationNode>(); });

    palette.RegisterNode("Initial Velocity", "Spawn", "ParticleVelocity",
                         "Applies initial velocity and directional angular spread",
                         []() { return CreateRef<ParticleVelocityNode>(); });

    palette.RegisterNode("Color Over Life", "Update", "ParticleColorOverLife",
                         "Animates particle RGBA color and opacity curves over lifetime",
                         []() { return CreateRef<ParticleColorOverLifeNode>(); });

    palette.RegisterNode("Size Over Life", "Update", "ParticleSizeOverLife",
                         "Animates particle scale over lifetime",
                         []() { return CreateRef<ParticleSizeOverLifeNode>(); });

    palette.RegisterNode("Turbulence Force", "Forces", "ParticleTurbulence",
                         "Applies dynamic curl noise / turbulence acceleration",
                         []() { return CreateRef<ParticleTurbulenceForceNode>(); });

    palette.RegisterNode("Physics Collision", "Forces", "ParticleCollision",
                         "Raycast obstacle collision and surface bounce",
                         []() { return CreateRef<ParticlePhysicsCollisionNode>(); });
}

TERef<GraphNode> ParticleGraphNodeRegistry::CreateNodeByType(const TEString &nodeType)
{
    if (nodeType == "ParticleEmitterRoot") return CreateRef<ParticleEmitterRootNode>();
    if (nodeType == "ParticleSpawnRate") return CreateRef<ParticleSpawnRateNode>();
    if (nodeType == "ParticleSpawnLocation") return CreateRef<ParticleSpawnLocationNode>();
    if (nodeType == "ParticleVelocity") return CreateRef<ParticleVelocityNode>();
    if (nodeType == "ParticleColorOverLife") return CreateRef<ParticleColorOverLifeNode>();
    if (nodeType == "ParticleSizeOverLife") return CreateRef<ParticleSizeOverLifeNode>();
    if (nodeType == "ParticleTurbulence") return CreateRef<ParticleTurbulenceForceNode>();
    if (nodeType == "ParticleCollision") return CreateRef<ParticlePhysicsCollisionNode>();

    return CreateRef<GraphNode>();
}
