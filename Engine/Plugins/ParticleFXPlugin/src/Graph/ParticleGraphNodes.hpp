#pragma once

#include "Core/Graph/GraphNode.hpp"
#include "Core/PreRequisites.h"

class ParticleEmitterRootNode : public GraphNode
{
public:
    ParticleEmitterRootNode()
    {
        NodeType = "ParticleEmitterRoot";
        Title = "Particle Emitter";
        Category = "Emitter";
        HeaderColor = TEColor(0.6f, 0.25f, 0.2f, 1.0f);
        Size = {210.0f, 160.0f};

        SetProperty("MaxParticles", "1000");
        SetProperty("BlendMode", "Additive"); // Normal, Additive, Multiply
        SetProperty("TexturePath", "");

        InputPins.Add(GraphPin(0, 0, "Spawn", GraphPinType::Custom("ParticleSpawn", TEColor(0.9f, 0.6f, 0.2f, 1.0f)),
                               PinDirection::Input));
        InputPins.Add(GraphPin(0, 0, "Update", GraphPinType::Custom("ParticleUpdate", TEColor(0.2f, 0.7f, 0.9f, 1.0f)),
                               PinDirection::Input));
        InputPins.Add(GraphPin(0, 0, "Forces", GraphPinType::Custom("ParticleForce", TEColor(0.8f, 0.3f, 0.8f, 1.0f)),
                               PinDirection::Input));
        InputPins.Add(GraphPin(0, 0, "Render", GraphPinType::Custom("ParticleRender", TEColor(0.3f, 0.8f, 0.4f, 1.0f)),
                               PinDirection::Input));
    }
};

class ParticleSpawnRateNode : public GraphNode
{
public:
    ParticleSpawnRateNode()
    {
        NodeType = "ParticleSpawnRate";
        Title = "Spawn Rate";
        Category = "Spawn";
        HeaderColor = TEColor(0.55f, 0.4f, 0.15f, 1.0f);
        Size = {180.0f, 110.0f};

        SetProperty("Rate", "50.0");
        SetProperty("BurstCount", "0");

        InputPins.Add(GraphPin(0, 0, "Rate", GraphPinType::Float(), PinDirection::Input, "50.0"));
        OutputPins.Add(GraphPin(0, 0, "Out", GraphPinType::Custom("ParticleSpawn", TEColor(0.9f, 0.6f, 0.2f, 1.0f)),
                                PinDirection::Output));
    }
};

class ParticleSpawnLocationNode : public GraphNode
{
public:
    ParticleSpawnLocationNode()
    {
        NodeType = "ParticleSpawnLocation";
        Title = "Spawn Location (Box/Sphere)";
        Category = "Spawn";
        HeaderColor = TEColor(0.5f, 0.45f, 0.2f, 1.0f);
        Size = {200.0f, 120.0f};

        SetProperty("Shape", "Box"); // Point, Box, Sphere, Circle
        SetProperty("ExtentsX", "1.0");
        SetProperty("ExtentsY", "1.0");

        InputPins.Add(GraphPin(0, 0, "Offset", GraphPinType::Float2(), PinDirection::Input, "0,0"));
        OutputPins.Add(GraphPin(0, 0, "Out", GraphPinType::Custom("ParticleSpawn", TEColor(0.9f, 0.6f, 0.2f, 1.0f)),
                                PinDirection::Output));
    }
};

class ParticleVelocityNode : public GraphNode
{
public:
    ParticleVelocityNode()
    {
        NodeType = "ParticleVelocity";
        Title = "Initial Velocity";
        Category = "Spawn";
        HeaderColor = TEColor(0.35f, 0.5f, 0.25f, 1.0f);
        Size = {190.0f, 120.0f};

        SetProperty("MinSpeed", "2.0");
        SetProperty("MaxSpeed", "5.0");
        SetProperty("AngleSpread", "45.0");

        InputPins.Add(GraphPin(0, 0, "Direction", GraphPinType::Float2(), PinDirection::Input, "0,1"));
        OutputPins.Add(GraphPin(0, 0, "Out", GraphPinType::Custom("ParticleSpawn", TEColor(0.9f, 0.6f, 0.2f, 1.0f)),
                                PinDirection::Output));
    }
};

class ParticleColorOverLifeNode : public GraphNode
{
public:
    ParticleColorOverLifeNode()
    {
        NodeType = "ParticleColorOverLife";
        Title = "Color Over Life";
        Category = "Update";
        HeaderColor = TEColor(0.2f, 0.45f, 0.6f, 1.0f);
        Size = {200.0f, 130.0f};

        SetProperty("StartColor", "1,0.8,0.2,1");
        SetProperty("EndColor", "1,0.1,0.0,0");

        OutputPins.Add(GraphPin(0, 0, "Out", GraphPinType::Custom("ParticleUpdate", TEColor(0.2f, 0.7f, 0.9f, 1.0f)),
                                PinDirection::Output));
    }
};

class ParticleSizeOverLifeNode : public GraphNode
{
public:
    ParticleSizeOverLifeNode()
    {
        NodeType = "ParticleSizeOverLife";
        Title = "Size Over Life";
        Category = "Update";
        HeaderColor = TEColor(0.25f, 0.5f, 0.55f, 1.0f);
        Size = {180.0f, 110.0f};

        SetProperty("StartSize", "1.0");
        SetProperty("EndSize", "0.0");

        OutputPins.Add(GraphPin(0, 0, "Out", GraphPinType::Custom("ParticleUpdate", TEColor(0.2f, 0.7f, 0.9f, 1.0f)),
                                PinDirection::Output));
    }
};

class ParticleTurbulenceForceNode : public GraphNode
{
public:
    ParticleTurbulenceForceNode()
    {
        NodeType = "ParticleTurbulence";
        Title = "Turbulence Force";
        Category = "Forces";
        HeaderColor = TEColor(0.55f, 0.25f, 0.55f, 1.0f);
        Size = {190.0f, 120.0f};

        SetProperty("Frequency", "1.5");
        SetProperty("Strength", "4.0");

        InputPins.Add(GraphPin(0, 0, "Strength", GraphPinType::Float(), PinDirection::Input, "4.0"));
        OutputPins.Add(GraphPin(0, 0, "Out", GraphPinType::Custom("ParticleForce", TEColor(0.8f, 0.3f, 0.8f, 1.0f)),
                                PinDirection::Output));
    }
};

class ParticlePhysicsCollisionNode : public GraphNode
{
public:
    ParticlePhysicsCollisionNode()
    {
        NodeType = "ParticleCollision";
        Title = "Physics Raycast Collision";
        Category = "Forces";
        HeaderColor = TEColor(0.6f, 0.25f, 0.35f, 1.0f);
        Size = {210.0f, 130.0f};

        SetProperty("Bounciness", "0.6");
        SetProperty("Friction", "0.1");
        SetProperty("KillOnContact", "false");

        OutputPins.Add(GraphPin(0, 0, "Out", GraphPinType::Custom("ParticleForce", TEColor(0.8f, 0.3f, 0.8f, 1.0f)),
                                PinDirection::Output));
    }
};
