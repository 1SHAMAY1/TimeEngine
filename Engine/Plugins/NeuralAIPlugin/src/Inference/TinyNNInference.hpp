#pragma once

#include "Core/PreRequisites.h"
#include "Utils/MathUtils.hpp"

class TinyNNInference
{
public:
    TinyNNInference() = default;
    ~TinyNNInference() = default;

    bool LoadFromMemory(const void *data, size_t size)
    {
        // TODO: Contributor implementation
        return true;
    }

    void Forward(const float *input, int inputSize, float *output, int outputSize)
    {
        // TODO: Contributor implementation - Matrix multiplication & activation
        for (int i = 0; i < outputSize; ++i)
        {
            output[i] = 0.0f;
        }
    }
};
