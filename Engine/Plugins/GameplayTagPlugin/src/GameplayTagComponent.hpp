#pragma once

#include "GameFrameWork/TComponent.hpp"
#include "GameplayTagContainer.hpp"
#include <string>

namespace TE
{

class GameplayTagComponent : public TComponent
{
public:
    GameplayTagContainer Container;

    GameplayTagComponent() = default;

    virtual const char *GetClassName() const override { return StaticClassName; }

    static constexpr const char *StaticClassName = "GameplayTagComponent";

    // Query tag hierarchy
    bool HasTag(const std::string &tagString) const { return Container.HasTag(tagString); }

    bool has_tag(const std::string &tagString) const { return HasTag(tagString); }

    bool HasTagExact(const std::string &tagString) const { return Container.HasTagExact(tagString); }

    void AddTag(const std::string &tagString) { Container.AddTag(tagString); }

    void RemoveTag(const std::string &tagString) { Container.RemoveTag(tagString); }

    virtual void OnDrawInspector() override
    {
        std::string tagList = Container.GetTagsAsString();
        TE::TimeGUI::Text(("Tags: " + (tagList.empty() ? "(None)" : tagList)).c_str());
    }
};

} // namespace TE
