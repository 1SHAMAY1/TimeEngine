#pragma once

#include <cmath>
#include <glm/glm.hpp>
#include <vector>

namespace IK
{

class FABRIKSolver3D
{
public:
    static bool Solve(TEArray<glm::vec3> &points, const TEArray<float> &lengths, const glm::vec3 &target,
                      int maxIterations = 15, float tolerance = 0.5f)
    {
        if (points.size() < 2 || lengths.size() != points.size() - 1)
            return false;

        glm::vec3 origin = points[0];
        float totalLength = 0.0f;
        for (float l : lengths)
            totalLength += l;

        float distToTarget = glm::distance(origin, target);

        // Case 1: Target unreachable -> stretch towards target
        if (distToTarget > totalLength)
        {
            glm::vec3 dir = glm::normalize(target - origin);
            for (size_t i = 0; i < lengths.size(); ++i)
            {
                points[i + 1] = points[i] + dir * lengths[i];
            }
            return true;
        }

        // Case 2: Target is reachable -> iterate Forward and Backward
        for (int iter = 0; iter < maxIterations; ++iter)
        {
            if (glm::distance(points.back(), target) < tolerance)
                break;

            // Forward Reaching: set end effector to target
            points.back() = target;
            for (int i = static_cast<int>(points.size()) - 2; i >= 0; --i)
            {
                glm::vec3 dir = glm::normalize(points[i] - points[i + 1]);
                points[i] = points[i + 1] + dir * lengths[i];
            }

            // Backward Reaching: restore root to origin
            points[0] = origin;
            for (size_t i = 0; i < lengths.size(); ++i)
            {
                glm::vec3 dir = glm::normalize(points[i + 1] - points[i]);
                points[i + 1] = points[i] + dir * lengths[i];
            }
        }

        return true;
    }
};

} // namespace IK
