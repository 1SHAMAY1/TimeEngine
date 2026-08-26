#pragma once

#include "IIKSolver.hpp"
#include <algorithm>
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <vector>

namespace IK
{

class CCDIKSolver3D
{
public:
    static bool Solve(TEArray<IKJoint3D> &joints, const glm::vec3 &target, int maxIterations = 10,
                      float tolerance = 0.5f)
    {
        if (joints.size() < 2)
            return false;

        for (int iter = 0; iter < maxIterations; ++iter)
        {
            glm::vec3 endEffector = joints.back().Position;
            if (glm::distance(endEffector, target) < tolerance)
                break;

            for (int i = static_cast<int>(joints.size()) - 2; i >= 0; --i)
            {
                glm::vec3 jointPos = joints[i].Position;
                glm::vec3 toEnd = endEffector - jointPos;
                glm::vec3 toTarget = target - jointPos;

                float lenEnd = glm::length(toEnd);
                float lenTarget = glm::length(toTarget);

                if (lenEnd > 0.0001f && lenTarget > 0.0001f)
                {
                    glm::vec3 uEnd = toEnd / lenEnd;
                    glm::vec3 uTarget = toTarget / lenTarget;

                    float cosAngle = glm::dot(uEnd, uTarget);
                    cosAngle = std::clamp(cosAngle, -1.0f, 1.0f);
                    float angle = std::acos(cosAngle);

                    if (angle > 0.001f)
                    {
                        glm::vec3 axis = glm::cross(uEnd, uTarget);
                        if (glm::length(axis) > 0.0001f)
                        {
                            axis = glm::normalize(axis);
                            glm::quat rot = glm::angleAxis(angle, axis);

                            // Rotate position of subsequent joints
                            for (size_t j = i + 1; j < joints.size(); ++j)
                            {
                                glm::vec3 offset = joints[j].Position - jointPos;
                                joints[j].Position = jointPos + rot * offset;
                            }
                            endEffector = joints.back().Position;
                        }
                    }
                }
            }
        }
        return true;
    }
};

} // namespace IK
