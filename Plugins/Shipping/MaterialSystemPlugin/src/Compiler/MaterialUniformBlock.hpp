#pragma once

#include "PreRequisites.h"
#include "GameplayUtils.hpp"
#include "MathUtils.hpp"
#include "EngineTypes/TEString.hpp"

struct ScalarParamDesc
{
    TEString Name;
    float DefaultValue = 0.0f;
    int Location = -1;
};

struct VectorParamDesc
{
    TEString Name;
    TEVector4 DefaultValue = {1.0f, 1.0f, 1.0f, 1.0f};
    int Location = -1;
};

struct TextureParamDesc
{
    TEString Name;
    TEString DefaultPath;
    int Slot = 0;
    int Location = -1;
};

struct MaterialUniformBlock
{
    TEMap<TEString, ScalarParamDesc> ScalarParameters;
    TEMap<TEString, VectorParamDesc> VectorParameters;
    TEMap<TEString, TextureParamDesc> TextureParameters;

    void Clear()
    {
        ScalarParameters.clear();
        VectorParameters.clear();
        TextureParameters.clear();
    }
};
