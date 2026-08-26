#pragma once

#include "Utils/MathUtils.hpp"
#include <algorithm>
#include <cmath>

namespace IK
{

struct FootPlacement2DResult
{
    TEVector2 FootTargetPosition = {0.0f, 0.0f};
    TEVector2 SurfaceNormal = {0.0f, 1.0f};
    float PelvisOffset = 0.0f;
    bool IsGrounded = false;
};

class FootGrounder2D
{
public:
    static FootPlacement2DResult Evaluate(const TEVector2 &defaultFootPos, float raycastHitY,
                                          const TEVector2 &groundNormal, float maxStepUp = 30.0f,
                                          float maxStepDown = 50.0f)
    {
        FootPlacement2DResult result;
        float heightDiff = raycastHitY - defaultFootPos.y;

        if (heightDiff >= -maxStepDown && heightDiff <= maxStepUp)
        {
            result.FootTargetPosition = defaultFootPos;
            result.FootTargetPosition.y = raycastHitY;
            result.SurfaceNormal = groundNormal;
            result.PelvisOffset = std::min(0.0f, heightDiff);
            result.IsGrounded = true;
        }
        else
        {
            result.FootTargetPosition = defaultFootPos;
            result.SurfaceNormal = TEVector2(0.0f, 1.0f);
            result.PelvisOffset = 0.0f;
            result.IsGrounded = false;
        }
        return result;
    }
};

} // namespace IK
