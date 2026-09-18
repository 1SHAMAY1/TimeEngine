#pragma once

#include "MathUtils.hpp"
#include <algorithm>
#include <cmath>

namespace IK
{

struct FootPlacementResult
{
    TEVector FootTargetPosition = {0.0f, 0.0f, 0.0f};
    TEVector SurfaceNormal = {0.0f, 1.0f, 0.0f};
    float PelvisOffset = 0.0f;
    bool IsGrounded = false;
};

class FootGrounder
{
public:
    static FootPlacementResult Evaluate(const TEVector &defaultFootPos, float raycastHitHeight,
                                        const TEVector &groundNormal, float maxStepUp = 30.0f,
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
            result.SurfaceNormal = TEVector(0.0f, 1.0f, 0.0f);
            result.PelvisOffset = 0.0f;
            result.IsGrounded = false;
        }
        return result;
    }
};

} // namespace IK
