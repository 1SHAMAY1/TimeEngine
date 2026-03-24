#pragma once
#include "CollisionTypes.hpp"
#include "GameFrameWork/TComponent.hpp"

namespace TE {

    class CollisionComponent : public TComponent {
    public:
        CollisionShape shape;
        bool isStatic = false;
        bool isTrigger = false;
        bool collided = false;

        virtual const char* GetClassName() const override { return StaticClassName; }
        static constexpr const char* StaticClassName = "CollisionComponent";

        virtual void OnUpdateShape(const glm::mat4& worldTransform) {}
    };

}
