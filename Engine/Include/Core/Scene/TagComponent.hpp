#pragma once
#include "GameFrameWork/TComponent.hpp"

class TagComponent : public TComponent
{
public:
    TEString Tag;

    TagComponent() = default;
    TagComponent(const TEString &tag) : Tag(tag) {}

    virtual TEString GetClassName() const override { return StaticClassName; }

    inline static const TEString StaticClassName = "TagComponent";
};

#ifdef TE_EDITOR
#include "Core/Scene/ComponentRegistry.hpp"
T_REGISTER_COMPONENT(TagComponent, "Tag Component")
T_REGISTER_PROPERTY(TagComponent, TEString, Tag, "Tag")
T_COMPONENT_INTERNAL(TagComponent)
#endif
