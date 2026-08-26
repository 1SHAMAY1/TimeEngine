#pragma once

#include <algorithm>
#include <cmath>
#include <glm/glm.hpp>

namespace IK
{

struct FootPlacementResult
{
    glm::vec3 FootTargetPosition = {0.0f, 0.0f, 0.0f};
    glm::vec3 SurfaceNormal = {0.0f, 1.0f, 0.0f};
    float PelvisOffset = 0.0f;
    bool IsGrounded = false;
};

class FootGrounder
{
public:
    static FootPlacementResult Evaluate(const glm::vec3 &defaultFootPos, float raycastHitHeight,
                                        const glm::vec3 &groundNormal, float maxStepUp = 30.0f,
                                        float maxStepDown = 50.0f)
    {
        FootPlacementResult result;
        float heightDiff = raycastHitHeight - defaultFootPos.y;

        if (heightDiff >= -maxStepDown && heightDiff <= maxStepUp)
        {
            result.FootTargetPosition = defaultFootPos;
            result.FootTargetPosition.y = raycastHitHeight;
            result.SurfaceNormal = groundNormal;
            result.PelvisOffset = std::min(0.0f, heightDiff);
            result.IsGrounded = true;
        }
        else
        {
            result.FootTargetPosition = defaultFootPos;
            result.SurfaceNormal = glm::vec3(0.0f, 1.0f, 0.0f);
            result.PelvisOffset = 0.0f;
            result.IsGrounded = false;
        }
        return result;
    }
};

} // namespace IK
