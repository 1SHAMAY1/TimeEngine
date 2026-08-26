#pragma once

#include "Core/PreRequisites.h"
#include "Utils/MathUtils.hpp"

class MaterialLighting2DBridge
{
public:
    static bool IsLighting2DActive();
    static void ApplyLightingPass(uint32_t shaderRendererID, const TEVector2 &lightPos, const TEVector4 &lightColor,
                                  float intensity);
};
