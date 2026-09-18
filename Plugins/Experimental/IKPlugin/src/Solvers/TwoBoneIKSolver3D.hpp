#pragma once

#include "IIKSolver.hpp"
#include "MathUtils.hpp"
#include <algorithm>
#include <cmath>

namespace IK
{

class TwoBoneIKSolver3D
{
public:
    static bool Solve(const TEVector &rootPos, float lengthA, float lengthB, const TEVector &targetPos,
                      const TEVector &poleTarget, TEVector &outMidPos)
    {
        TEVector toTarget = targetPos - rootPos;
        float dist = toTarget.Length();
        if (dist < 0.0001f)
            return false;

        float maxDist = lengthA + lengthB;
        float minDist = std::abs(lengthA - lengthB);
        float clampedDist = std::clamp(dist, minDist + 0.0001f, maxDist - 0.0001f);

        // Law of cosines for angle at root (alpha)
        float cosAlpha =
            (lengthA * lengthA + clampedDist * clampedDist - lengthB * lengthB) / (2.0f * lengthA * clampedDist);
        cosAlpha = std::clamp(cosAlpha, -1.0f, 1.0f);
        float alpha = std::acos(cosAlpha);

        TEVector targetDir = toTarget.Normalized();
        TEVector poleDir = poleTarget - rootPos;

        // Plane normal defined by root->target and pole vector
        TEVector planeNormal = targetDir.Cross(poleDir);
        if (planeNormal.Length() < 0.0001f)
        {
            // Fallback plane normal
            planeNormal = targetDir.Cross(TEVector(0.0f, 1.0f, 0.0f));
            if (planeNormal.Length() < 0.0001f)
                planeNormal = targetDir.Cross(TEVector(1.0f, 0.0f, 0.0f));
        }
        planeNormal = planeNormal.Normalized();

        // Bend direction in plane
        TEVector bendDir = planeNormal.Cross(targetDir).Normalized();

        // Mid point position
        outMidPos = rootPos + (targetDir * (std::cos(alpha) * lengthA)) + (bendDir * (std::sin(alpha) * lengthA));
        return true;
    }
};

} // namespace IK
