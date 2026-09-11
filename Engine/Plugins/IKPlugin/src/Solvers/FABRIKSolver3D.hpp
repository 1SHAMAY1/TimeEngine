#pragma once

#include "Utils/MathUtils.hpp"
#include <cmath>

namespace IK
{

class FABRIKSolver3D
{
public:
    static bool Solve(TEArray<TEVector> &points, const TEArray<float> &lengths, const TEVector &target,
                      int maxIterations = 15, float tolerance = 0.5f)
    {
        if (points.Num() < 2 || lengths.Num() != points.Num() - 1)
            return false;

        TEVector origin = points[0];
        float totalLength = 0.0f;
        for (size_t i = 0; i < lengths.Num(); ++i)
            totalLength += lengths[i];

        float distToTarget = (target - origin).Length();

        // Case 1: Target unreachable -> stretch towards target
        if (distToTarget > totalLength)
        {
            TEVector dir = (target - origin).Normalized();
            for (size_t i = 0; i < lengths.Num(); ++i)
            {
                points[i + 1] = points[i] + dir * lengths[i];
            }
            return true;
        }

        // Case 2: Target is reachable -> iterate Forward and Backward
        for (int iter = 0; iter < maxIterations; ++iter)
        {
            if ((points[points.Num() - 1] - target).Length() < tolerance)
                break;

            // Forward Reaching: set end effector to target
            points[points.Num() - 1] = target;
            for (int i = static_cast<int>(points.Num()) - 2; i >= 0; --i)
            {
                TEVector dir = (points[i] - points[i + 1]).Normalized();
                points[i] = points[i + 1] + dir * lengths[i];
            }

            // Backward Reaching: restore root to origin
            points[0] = origin;
            for (size_t i = 0; i < lengths.Num(); ++i)
            {
                TEVector dir = (points[i + 1] - points[i]).Normalized();
                points[i + 1] = points[i] + dir * lengths[i];
            }
        }

        return true;
    }
};

} // namespace IK
