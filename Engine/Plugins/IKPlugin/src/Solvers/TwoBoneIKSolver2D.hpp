#pragma once

#include <cmath>
#include <algorithm>
#include "Utils/MathUtils.hpp"
#include "IIKSolver.hpp"

namespace IK {

class TwoBoneIKSolver2D
{
public:
    static bool Solve(const TEVector2& rootPos, float lengthA, float lengthB,
                      const TEVector2& targetPos, bool bendPositive,
                      TEVector2& outMidPos, float& outAngleA, float& outAngleB)
    {
        TEVector2 toTarget = targetPos - rootPos;
        float dist = toTarget.Length();
        if (dist < 0.0001f)
            return false;

        float maxDist = lengthA + lengthB;
        float minDist = std::abs(lengthA - lengthB);

        // Clamp reach
        float clampedDist = std::clamp(dist, minDist + 0.0001f, maxDist - 0.0001f);

        // Law of Cosines: c^2 = a^2 + b^2 - 2ab * cos(gamma)
        float cosAngleA = (lengthA * lengthA + clampedDist * clampedDist - lengthB * lengthB) / (2.0f * lengthA * clampedDist);
        cosAngleA = std::clamp(cosAngleA, -1.0f, 1.0f);
        float angleOffsetA = std::acos(cosAngleA);

        float cosAngleB = (lengthA * lengthA + lengthB * lengthB - clampedDist * clampedDist) / (2.0f * lengthA * lengthB);
        cosAngleB = std::clamp(cosAngleB, -1.0f, 1.0f);
        float angleB = std::acos(cosAngleB);

        float baseAngle = std::atan2(toTarget.y, toTarget.x);
        outAngleA = bendPositive ? (baseAngle + angleOffsetA) : (baseAngle - angleOffsetA);
        outAngleB = bendPositive ? (outAngleA - (3.14159265f - angleB)) : (outAngleA + (3.14159265f - angleB));

        outMidPos = rootPos + TEVector2(std::cos(outAngleA), std::sin(outAngleA)) * lengthA;
        return true;
    }
};

} // namespace IK
