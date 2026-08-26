#pragma once
#include "Core/PreRequisites.h"
#include "GameFrameWork/GameplayUtils.hpp"
#include "Utils/MathUtils.hpp"
#include <algorithm>

enum class EEmissionShape : uint8_t
{
    Point = 0,
    Circle2D,
    Box2D,
    Cone2D,
    Sphere3D,
    Box3D
};

enum class ESimulationSpace : uint8_t
{
    World = 0,
    Local
};

enum class EParticleBlendMode : uint8_t
{
    AlphaBlend = 0,
    Additive,
    Multiply,
    Opaque
};

enum class EParticleAlignment : uint8_t
{
    ScreenAligned = 0,
    VelocityAligned,
    Custom
};

struct ColorGradientStop
{
    float Position = 0.0f; // 0.0 to 1.0
    TEVector4 Color = {1.0f, 1.0f, 1.0f, 1.0f};
};

struct ColorGradient
{
    TEArray<ColorGradientStop> Stops = {{0.0f, {1.0f, 1.0f, 1.0f, 1.0f}}, {1.0f, {1.0f, 1.0f, 1.0f, 0.0f}}};

    TEVector4 Evaluate(float t) const
    {
        if (Stops.IsEmpty())
            return {1.0f, 1.0f, 1.0f, 1.0f};

        t = std::clamp(t, 0.0f, 1.0f);
        if (t <= Stops.front().Position)
            return Stops.front().Color;
        if (t >= Stops.back().Position)
            return Stops.back().Color;

        for (size_t i = 0; i < Stops.Num() - 1; ++i)
        {
            if (t >= Stops[i].Position && t <= Stops[i + 1].Position)
            {
                float segmentLen = Stops[i + 1].Position - Stops[i].Position;
                float factor = (segmentLen > 0.00001f) ? (t - Stops[i].Position) / segmentLen : 0.0f;
                return Stops[i].Color * (1.0f - factor) + Stops[i + 1].Color * factor;
            }
        }
        return Stops.back().Color;
    }
};

struct FloatCurveKey
{
    float Time = 0.0f; // 0.0 to 1.0
    float Value = 1.0f;
};

struct FloatCurve
{
    TEArray<FloatCurveKey> Keys = {{0.0f, 1.0f}, {1.0f, 0.0f}};

    float Evaluate(float t) const
    {
        if (Keys.IsEmpty())
            return 1.0f;

        t = std::clamp(t, 0.0f, 1.0f);
        if (t <= Keys.front().Time)
            return Keys.front().Value;
        if (t >= Keys.back().Time)
            return Keys.back().Value;

        for (size_t i = 0; i < Keys.size() - 1; ++i)
        {
            if (t >= Keys[i].Time && t <= Keys[i + 1].Time)
            {
                float segmentLen = Keys[i + 1].Time - Keys[i].Time;
                float factor = (segmentLen > 0.00001f) ? (t - Keys[i].Time) / segmentLen : 0.0f;
                return Keys[i].Value * (1.0f - factor) + Keys[i + 1].Value * factor;
            }
        }
        return Keys.back().Value;
    }
};

struct BurstConfig
{
    uint32_t Count = 20;
    float Interval = 1.0f;
    float Delay = 0.0f;
    int32_t Cycles = 1; // -1 = Infinite
    int32_t CompletedCycles = 0;
    float Timer = 0.0f;
    bool Enabled = true;
};

struct Particle
{
    TEVector Position = {0.0f, 0.0f, 0.0f};
    TEVector Velocity = {0.0f, 0.0f, 0.0f};
    TEVector Acceleration = {0.0f, 0.0f, 0.0f};

    TEVector4 Color = {1.0f, 1.0f, 1.0f, 1.0f};
    TEVector4 BaseColor = {1.0f, 1.0f, 1.0f, 1.0f};

    float Size = 1.0f;
    float BaseSize = 1.0f;
    float Lifetime = 1.0f;
    float MaxLifetime = 1.0f;
    float Rotation = 0.0f;
    float AngularVelocity = 0.0f;
    float Drag = 0.0f;
    float Bounciness = 0.5f;

    bool Active = false;
    bool PhysicsCollidable = false;
};
