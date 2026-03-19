#pragma once
#include "GameFrameWork/TComponent.hpp"
#include "Utility/MathUtils.hpp"
#include "Renderer/TEColor.hpp"

namespace TE {

    class ProceduralSpriteComponent : public TComponent {
    public:
        TEColor BaseColor = TEColor::White();
        bool bIsVisible = true;

        virtual const char* GetClassName() const override { return StaticClassName; }

        static constexpr const char* StaticClassName = "ProceduralSpriteComponent";
    };

} // namespace TE
