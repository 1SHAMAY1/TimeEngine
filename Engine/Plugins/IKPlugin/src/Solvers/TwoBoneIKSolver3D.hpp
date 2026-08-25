#pragma once

#include <cmath>
#include <algorithm>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include "IIKSolver.hpp"

namespace IK {

class TwoBoneIKSolver3D
{
public:
    static bool Solve(const glm::vec3& rootPos, float lengthA, float lengthB,
                      const glm::vec3& targetPos, const glm::vec3& poleTarget,
                      glm::vec3& outMidPos)
    {
        glm::vec3 toTarget = targetPos - rootPos;
        float dist = glm::length(toTarget);
        if (dist < 0.0001f)
            return false;

        float maxDist = lengthA + lengthB;
        float minDist = std::abs(lengthA - lengthB);
        float clampedDist = std::clamp(dist, minDist + 0.0001f, maxDist - 0.0001f);

        // Law of cosines for angle at root (alpha)
        float cosAlpha = (lengthA * lengthA + clampedDist * clampedDist - lengthB * lengthB) / (2.0f * lengthA * clampedDist);
        cosAlpha = std::clamp(cosAlpha, -1.0f, 1.0f);
        float alpha = std::acos(cosAlpha);

        glm::vec3 targetDir = glm::normalize(toTarget);
        glm::vec3 poleDir = poleTarget - rootPos;

        // Plane normal defined by root->target and pole vector
        glm::vec3 planeNormal = glm::cross(targetDir, poleDir);
        if (glm::length(planeNormal) < 0.0001f)
        {
            // Fallback plane normal
            planeNormal = glm::cross(targetDir, glm::vec3(0.0f, 1.0f, 0.0f));
            if (glm::length(planeNormal) < 0.0001f)
                planeNormal = glm::cross(targetDir, glm::vec3(1.0f, 0.0f, 0.0f));
        }
        planeNormal = glm::normalize(planeNormal);

        // Bend direction in plane
        glm::vec3 bendDir = glm::normalize(glm::cross(planeNormal, targetDir));

        // Mid point position
        outMidPos = rootPos + (targetDir * (std::cos(alpha) * lengthA)) + (bendDir * (std::sin(alpha) * lengthA));
        return true;
    }
};

} // namespace IK
