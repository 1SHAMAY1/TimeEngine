#pragma once
#include "GameFrameWork/TComponent.hpp"
#include <string>

namespace TE {

    class TagComponent : public TComponent {
    public:
        std::string Tag;

        TagComponent() = default;
        TagComponent(const std::string& tag) : Tag(tag) {}

        virtual const char* GetClassName() const override { return StaticClassName; }

        static constexpr const char* StaticClassName = "TagComponent";
    };

} // namespace TE
