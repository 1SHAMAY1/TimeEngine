#include "Integration/MaterialLighting2DBridge.hpp"
#include "Core/Log.h"


bool MaterialLighting2DBridge::IsLighting2DActive()
{
#if defined(TE_PLUGIN_LIGHTING2D_ENABLED)
    return true;
#else
    return false;
#endif
}

void MaterialLighting2DBridge::ApplyLightingPass(uint32_t shaderRendererID, const TEVector2 &lightPos,
                                                 const TEVector4 &lightColor, float intensity)
{
#if defined(TE_PLUGIN_LIGHTING2D_ENABLED)
    // Pass dynamic 2D lighting uniforms to shader
#endif
}
