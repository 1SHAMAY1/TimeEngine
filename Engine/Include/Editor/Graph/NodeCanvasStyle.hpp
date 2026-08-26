#pragma once

#include "Core/Graph/GraphPin.hpp"
#include "Core/PreRequisites.h"
#include "Renderer/TEColor.hpp"
#include "Utils/MathUtils.hpp"

struct TE_API NodeCanvasStyle
{
    float GridSpacing = 32.0f;
    TEColor GridColor = {0.18f, 0.18f, 0.20f, 0.4f};
    TEColor GridColorMajor = {0.24f, 0.24f, 0.28f, 0.6f};
    TEColor BackgroundColor = {0.12f, 0.12f, 0.14f, 1.0f};

    TEColor NodeBgColor = {0.16f, 0.16f, 0.18f, 0.95f};
    TEColor NodeSelectedBorderColor = {0.95f, 0.65f, 0.2f, 1.0f};
    TEColor NodeBorderColor = {0.28f, 0.28f, 0.32f, 1.0f};

    float NodeRounding = 6.0f;
    float NodeBorderWidth = 1.5f;
    float WireThickness = 2.5f;
    float PinRadius = 5.5f;

    static TEColor GetPinColor(const GraphPinType &type) { return type.GetColor(); }
};
