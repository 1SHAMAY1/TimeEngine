#pragma once

#include "Utils/MathUtils.hpp"
#include "Utils/TEString.hpp"

namespace IK
{

enum class IKSolverType
{
    TwoBone2D,
    FABRIK2D,
    CCD2D,
    Aim2D,
    FootGrounding2D
};

struct IKJoint2D
{
    TEVector2 Position = {0.0f, 0.0f};
    float Angle = 0.0f; // Radians
    float Length = 50.0f;
    float MinAngle = -3.14159f;
    float MaxAngle = 3.14159f;
};

} // namespace IK
