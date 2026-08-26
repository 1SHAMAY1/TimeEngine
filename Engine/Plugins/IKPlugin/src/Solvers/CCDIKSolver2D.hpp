#pragma once

#include "IIKSolver.hpp"
#include "Utils/MathUtils.hpp"
#include <algorithm>
#include <cmath>

namespace IK
{

class CCDIKSolver2D
{
public:
    static bool Solve(TEArray<IKJoint2D> &joints, const TEVector2 &target, int maxIterations = 10,
                      float tolerance = 0.5f)
    {
        if (joints.size() < 2)
            return false;

        for (int iter = 0; iter < maxIterations; ++iter)
        {
            TEVector2 endEffector = joints.back().Position;
            if (Distance(endEffector, target) < tolerance)
                break;

            for (int i = static_cast<int>(joints.size()) - 2; i >= 0; --i)
            {
                TEVector2 jointPos = joints[i].Position;
                TEVector2 toEnd = endEffector - jointPos;
                TEVector2 toTarget = target - jointPos;

                float lenEnd = toEnd.Length();
                float lenTarget = toTarget.Length();

                if (lenEnd > 0.0001f && lenTarget > 0.0001f)
                {
                    float cosAngle = Dot(toEnd, toTarget) / (lenEnd * lenTarget);
                    cosAngle = std::clamp(cosAngle, -1.0f, 1.0f);
                    float angle = std::acos(cosAngle);

                    // Cross product to determine 2D sign
                    float crossZ = toEnd.x * toTarget.y - toEnd.y * toTarget.x;
                    if (crossZ < 0.0f)
                        angle = -angle;

                    joints[i].Angle += angle;
                    joints[i].Angle = std::clamp(joints[i].Angle, joints[i].MinAngle, joints[i].MaxAngle);

                    // Forward kinematics update from this joint down
                    for (size_t j = i + 1; j < joints.size(); ++j)
                    {
                        float a = joints[j - 1].Angle;
                        joints[j].Position =
                            joints[j - 1].Position + TEVector2(std::cos(a), std::sin(a)) * joints[j - 1].Length;
                    }
                    endEffector = joints.back().Position;
                }
            }
        }
        return true;
    }
};

} // namespace IK
