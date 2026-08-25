#pragma once

#include <cmath>
#include "Utils/MathUtils.hpp"
#include "GameFrameWork/GameplayUtils.hpp"

namespace IK {

class FABRIKSolver2D
{
public:
    static bool Solve(TEArray<TEVector2>& points, const TEArray<float>& lengths,
                      const TEVector2& target, int maxIterations = 15, float tolerance = 0.5f)
    {
        if (points.size() < 2 || lengths.size() != points.size() - 1)
            return false;

        TEVector2 origin = points[0];
        float totalLength = 0.0f;
        for (float l : lengths) totalLength += l;

        float distToTarget = Distance(origin, target);

        // Case 1: Target unreachable -> stretch towards target
        if (distToTarget > totalLength)
        {
            TEVector2 dir = (target - origin).Normalized();
            for (size_t i = 0; i < lengths.size(); ++i)
            {
                points[i + 1] = points[i] + dir * lengths[i];
            }
            return true;
        }

        // Case 2: Target is reachable -> iterate Forward and Backward
        for (int iter = 0; iter < maxIterations; ++iter)
        {
            if (Distance(points.back(), target) < tolerance)
                break;

            // Forward Reaching: set end effector to target
            points.back() = target;
            for (int i = static_cast<int>(points.size()) - 2; i >= 0; --i)
            {
                TEVector2 dir = (points[i] - points[i + 1]).Normalized();
                points[i] = points[i + 1] + dir * lengths[i];
            }

            // Backward Reaching: restore root to origin
            points[0] = origin;
            for (size_t i = 0; i < lengths.size(); ++i)
            {
                TEVector2 dir = (points[i + 1] - points[i]).Normalized();
                points[i + 1] = points[i] + dir * lengths[i];
            }
        }

        return true;
    }
};

} // namespace IK
