#pragma once

#include "IIKSolver.hpp"
#include "MathUtils.hpp"
#include <algorithm>
#include <cmath>

namespace IK
{

class CCDIKSolver3D
{
public:
    static bool Solve(TEArray<IKJoint3D> &joints, const TEVector &target, int maxIterations = 10,
                      float tolerance = 0.5f)
    {
        if (joints.Num() < 2)
            return false;

        for (int iter = 0; iter < maxIterations; ++iter)
        {
            TEVector endEffector = joints[joints.Num() - 1].Position;
            float dist = (endEffector - target).Length();
            if (dist < tolerance)
                break;

            for (int i = static_cast<int>(joints.Num()) - 2; i >= 0; --i)
            {
                TEVector jointPos = joints[i].Position;
                TEVector toEnd = endEffector - jointPos;
                TEVector toTarget = target - jointPos;

                float lenEnd = toEnd.Length();
                float lenTarget = toTarget.Length();

                if (lenEnd > 0.0001f && lenTarget > 0.0001f)
                {
                    TEVector uEnd = toEnd / lenEnd;
                    TEVector uTarget = toTarget / lenTarget;

                    float cosAngle = uEnd.Dot(uTarget);
                    cosAngle = std::clamp(cosAngle, -1.0f, 1.0f);
                    float angle = std::acos(cosAngle);

                    if (angle > 0.001f)
                    {
                        TEVector axis = uEnd.Cross(uTarget);
                        if (axis.Length() > 0.0001f)
                        {
                            axis = axis.Normalized();
                            TEQuat rot = TEQuat::AngleAxis(angle, axis);

                            // Rotate position of subsequent joints
                            for (size_t j = i + 1; j < joints.Num(); ++j)
                            {
                                TEVector offset = joints[j].Position - jointPos;
                                joints[j].Position = jointPos + rot * offset;
                            }
                            endEffector = joints[joints.Num() - 1].Position;
                        }
                    }
                }
            }
        }
        return true;
    }
};

} // namespace IK
