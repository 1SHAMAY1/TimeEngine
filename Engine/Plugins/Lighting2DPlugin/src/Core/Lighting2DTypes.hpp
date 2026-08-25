#pragma once

#include "Core/Math/Math.hpp"
#include "Renderer/TEColor.hpp"
#include <cstdint>


enum class ELight2DType : uint8_t
{
    Point = 0,
    Spot = 1,
    Directional = 2,
    Line = 3,
    Area = 4,
    FreeformPolygon = 5
};

enum class EShadowCasterType : uint8_t
{
    BoundingBox = 0,
    Circle = 1,
    PolygonPoints = 2,
    TilemapCollider = 3
};

enum class ELightingTechnique : uint8_t
{
    ShadowMap1D = 0,    // Polar 1D coordinate shadow mapping (Mobile / Fast)
    SDFRayTracing = 1,  // GPU Signed Distance Field (Jump Flood) Ray Marching (Contact Hardening Soft Shadows)
    Radiosity2D = 2     // 2D Path Traced Multi-Bounce Indirect Radiance & Color Bleed (Cinematic GI)
};

struct Light2DData
{
    ELight2DType Type = ELight2DType::Point;
    TEVector2 Position = {0.0f, 0.0f};
    TEColor Color = TEColor::White();
    float Intensity = 1.0f;
    float Radius = 8.0f;
    float InnerAngle = 20.0f;
    float OuterAngle = 45.0f;
    TEVector2 Direction = {0.0f, -1.0f};
    float SourceRadius = 0.25f; // Controls penumbra softness
    float ShadowStrength = 1.0f;
    float VolumetricIntensity = 0.0f;
};

