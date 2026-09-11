#pragma once

#include "Utils/Math/MathEngine.hpp"
#include "Utils/MathUtils.hpp"
#include <algorithm>
#include <cmath>

namespace IK
{

class AimConstraint3D
{
public:
    static TEQuat Solve(const TEVector &boneWorldPos, const TEVector &targetWorldPos,
                        const TEVector &aimAxis = TEVector(0.0f, 0.0f, 1.0f),
                        const TEVector &upVector = TEVector(0.0f, 1.0f, 0.0f))
    {
        TEVector toTarget = targetWorldPos - boneWorldPos;
        if (toTarget.Length() < 0.0001f)
            return TEQuat(1.0f, 0.0f, 0.0f, 0.0f);

        TEVector forward = toTarget.Normalized();
        TEVector right = upVector.Cross(forward);
        if (right.Length() < 0.0001f)
        {
            right = TEVector(1.0f, 0.0f, 0.0f).Cross(forward);
        }
        right = right.Normalized();
        TEVector up = forward.Cross(right).Normalized();

        TEMatrix4 rotMat(1.0f);
        rotMat.m[0][0] = right.x;
        rotMat.m[0][1] = right.y;
        rotMat.m[0][2] = right.z;

        rotMat.m[1][0] = up.x;
        rotMat.m[1][1] = up.y;
        rotMat.m[1][2] = up.z;

        rotMat.m[2][0] = forward.x;
        rotMat.m[2][1] = forward.y;
        rotMat.m[2][2] = forward.z;

        return TEQuat::FromMatrix(rotMat);
    }
};

} // namespace IK
