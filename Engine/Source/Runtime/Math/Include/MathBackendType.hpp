#pragma once
#include "PreRequisites.h"

/// Enumeration of supported math computation backends.
enum class MathBackendType
{
    GLM = 0,   ///< GLM-based math operations
    Custom = 1 ///< High-performance custom math library (SIMD/NEON/AVX2)
};
