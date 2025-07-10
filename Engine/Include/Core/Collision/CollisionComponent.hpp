#pragma once
#include "CollisionTypes.hpp"

namespace TE {

    struct CollisionComponent {
        CollisionShape shape;
        bool isStatic = false;
        bool isTrigger = false;
        bool collided = false;
    };

}
