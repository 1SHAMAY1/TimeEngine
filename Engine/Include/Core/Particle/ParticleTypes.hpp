#pragma once
#include "Utility/MathUtils.hpp"

namespace TE {

struct Particle {
    TEVector Position;
    TEVector Velocity;
    TEVector Acceleration;
    TEVector4 Color;
    float Size = 1.0f;
    float Lifetime = 1.0f;
    float MaxLifetime = 1.0f;
    float Rotation = 0.0f;
    bool Active = false;
};

}
