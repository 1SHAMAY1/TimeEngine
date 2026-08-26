#pragma once

#include "Utils/MathUtils.hpp"
#include <algorithm>
#include <cmath>

namespace IK
{

class AimConstraint2D
{
public:
    static float Solve(const TEVector2 &boneWorldPos, const TEVector2 &targetWorldPos, float currentRotation,
                       float minAngle, float maxAngle, float maxSpeed = 10.0f, float dt = 0.016f)
    {
        TEVector2 diff = targetWorldPos - boneWorldPos;
        float targetAngle = std::atan2(diff.y, diff.x);

        // Clamp to allowed angle arc
        targetAngle = std::clamp(targetAngle, minAngle, maxAngle);

        // Smooth angle step
        float diffAngle = targetAngle - currentRotation;
        while (diffAngle > 3.14159265f)
            diffAngle -= 6.2831853f;
        while (diffAngle < -3.14159265f)
            diffAngle += 6.2831853f;

        float maxStep = maxSpeed * dt;
        diffAngle = std::clamp(diffAngle, -maxStep, maxStep);
        return currentRotation + diffAngle;
    }
};

} // namespace IK
