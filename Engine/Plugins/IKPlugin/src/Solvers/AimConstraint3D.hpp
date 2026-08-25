#pragma once

#include <cmath>
#include <algorithm>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace IK {

class AimConstraint3D
{
public:
    static glm::quat Solve(const glm::vec3& boneWorldPos, const glm::vec3& targetWorldPos,
                           const glm::vec3& aimAxis = glm::vec3(0.0f, 0.0f, 1.0f),
                           const glm::vec3& upVector = glm::vec3(0.0f, 1.0f, 0.0f))
    {
        glm::vec3 toTarget = targetWorldPos - boneWorldPos;
        if (glm::length(toTarget) < 0.0001f)
            return glm::quat(1.0f, 0.0f, 0.0f, 0.0f);

        glm::vec3 forward = glm::normalize(toTarget);
        glm::vec3 right = glm::cross(upVector, forward);
        if (glm::length(right) < 0.0001f)
        {
            right = glm::cross(glm::vec3(1.0f, 0.0f, 0.0f), forward);
        }
        right = glm::normalize(right);
        glm::vec3 up = glm::normalize(glm::cross(forward, right));

        glm::mat3 rotMat(right, up, forward);
        return glm::quat_cast(rotMat);
    }
};

} // namespace IK
