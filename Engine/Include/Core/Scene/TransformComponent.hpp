#pragma once
#include "GameFrameWork/TComponent.hpp"
#include "Utility/MathUtils.hpp"
#include <vector>

namespace TE {

    class TransformComponent : public TComponent {
    public:
        
        EntityID Parent = 0;
        std::vector<EntityID> Children;

        TransformComponent() = default;
        TransformComponent(const TEVector& position) { Transform.Position = position.ToGLM(); }

        virtual const char* GetClassName() const override { return StaticClassName; }

        static constexpr const char* StaticClassName = "TransformComponent";
    };

} // namespace TE
