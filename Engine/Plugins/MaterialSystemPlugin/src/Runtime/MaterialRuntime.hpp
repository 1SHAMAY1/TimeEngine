#pragma once

#include "Core/PreRequisites.h"
#include "Asset/MaterialAsset.hpp"
#include "Asset/MaterialInstanceAsset.hpp"


class MaterialRuntime
{
public:
    static void BindMaterial(const TERef<MaterialAsset> &material, float time = 0.0f,
                             const TEVector2 &lightPos = {0.0f, 0.0f},
                             const TEVector4 &lightColor = {1.0f, 1.0f, 1.0f, 1.0f},
                             float lightIntensity = 1.0f);

    static void BindMaterialInstance(const TERef<MaterialInstanceAsset> &instance,
                                     const TERef<MaterialAsset> &parentMaterial,
                                     float time = 0.0f,
                                     const TEVector2 &lightPos = {0.0f, 0.0f},
                                     const TEVector4 &lightColor = {1.0f, 1.0f, 1.0f, 1.0f},
                                     float lightIntensity = 1.0f);
};
