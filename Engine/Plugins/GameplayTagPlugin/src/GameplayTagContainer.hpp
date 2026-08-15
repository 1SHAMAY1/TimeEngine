#pragma once

#include "GameplayTag.hpp"
#include <algorithm>
#include <sstream>
#include <string>
#include <unordered_set>
#include <vector>

namespace TE
{

class GameplayTagContainer
{
public:
    GameplayTagContainer() = default;

    void AddTag(const GameplayTag &tag)
    {
        if (tag.IsValid())
        {
            m_Tags.insert(tag);
        }
    }

    void AddTag(const std::string &tagString) { AddTag(GameplayTag(tagString)); }

    bool RemoveTag(const GameplayTag &tag) { return m_Tags.erase(tag) > 0; }

    bool RemoveTag(const std::string &tagString) { return RemoveTag(GameplayTag(tagString)); }

    void Clear() { m_Tags.clear(); }

    bool IsEmpty() const { return m_Tags.empty(); }

    size_t Size() const { return m_Tags.size(); }

    // Returns true if any tag in this container matches or is a child of tagQuery
    // Example: Container has "Character.Enemy.Boss.Dragon". HasTag("Character.Enemy") -> true
    bool HasTag(const GameplayTag &tagQuery) const
    {
        if (!tagQuery.IsValid())
            return false;

        for (const auto &tag : m_Tags)
        {
            if (tag.MatchesTag(tagQuery))
                return true;
        }

        return false;
    }

    bool HasTag(const std::string &tagQueryString) const { return HasTag(GameplayTag(tagQueryString)); }

    bool HasTagExact(const GameplayTag &tagQuery) const { return m_Tags.find(tagQuery) != m_Tags.end(); }

    bool HasTagExact(const std::string &tagQueryString) const { return HasTagExact(GameplayTag(tagQueryString)); }

    bool HasAllTags(const GameplayTagContainer &other) const
    {
        for (const auto &tag : other.m_Tags)
        {
            if (!HasTag(tag))
                return false;
        }
        return true;
    }

    bool HasAnyTags(const GameplayTagContainer &other) const
    {
        for (const auto &tag : other.m_Tags)
        {
            if (HasTag(tag))
                return true;
        }
        return false;
    }

    std::vector<GameplayTag> GetTags() const { return std::vector<GameplayTag>(m_Tags.begin(), m_Tags.end()); }

    std::string GetTagsAsString() const
    {
        std::stringstream ss;
        bool first = true;
        for (const auto &tag : m_Tags)
        {
            if (!first)
                ss << ", ";
            ss << tag.ToString();
            first = false;
        }
        return ss.str();
    }

private:
    std::unordered_set<GameplayTag> m_Tags;
};

} // namespace TE
