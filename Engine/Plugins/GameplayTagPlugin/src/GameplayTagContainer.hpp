#pragma once

#include "Core/PreRequisites.h"
#include "GameFrameWork/GameplayUtils.hpp"
#include "GameplayTag.hpp"
#include <algorithm>

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

    void AddTag(const TEString &tagString) { AddTag(GameplayTag(tagString)); }

    bool RemoveTag(const GameplayTag &tag) { return m_Tags.erase(tag) > 0; }

    bool RemoveTag(const TEString &tagString) { return RemoveTag(GameplayTag(tagString)); }

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

    bool HasTag(const TEString &tagQueryString) const { return HasTag(GameplayTag(tagQueryString)); }

    bool HasTagExact(const GameplayTag &tagQuery) const { return m_Tags.find(tagQuery) != m_Tags.end(); }

    bool HasTagExact(const TEString &tagQueryString) const { return HasTagExact(GameplayTag(tagQueryString)); }

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

    void AddTags(const GameplayTagContainer &other)
    {
        for (const auto &tag : other.m_Tags)
        {
            AddTag(tag);
        }
    }

    GameplayTagContainer &operator+=(const GameplayTagContainer &other)
    {
        AddTags(other);
        return *this;
    }

    GameplayTagContainer &operator+=(const GameplayTag &tag)
    {
        AddTag(tag);
        return *this;
    }

    void RemoveTags(const GameplayTagContainer &other)
    {
        for (const auto &tag : other.m_Tags)
        {
            RemoveTag(tag);
        }
    }

    GameplayTagContainer &operator-=(const GameplayTagContainer &other)
    {
        RemoveTags(other);
        return *this;
    }

    GameplayTagContainer &operator-=(const GameplayTag &tag)
    {
        RemoveTag(tag);
        return *this;
    }

    // Toggles a tag: if present, removes it; if absent, adds it. Returns new state (true if added, false if removed).
    bool ToggleTag(const GameplayTag &tag)
    {
        if (HasTagExact(tag))
        {
            RemoveTag(tag);
            return false;
        }
        else
        {
            AddTag(tag);
            return true;
        }
    }

    bool ToggleTag(const TEString &tagString) { return ToggleTag(GameplayTag(tagString)); }

    void AppendTags(const TEArray<TEString> &tagList)
    {
        for (const auto &str : tagList)
        {
            AddTag(str);
        }
    }

    // Returns a new container containing only the tags in this container that match or are children of queryTag
    GameplayTagContainer Filter(const GameplayTag &queryTag) const
    {
        GameplayTagContainer result;
        for (const auto &tag : m_Tags)
        {
            if (tag.MatchesTag(queryTag))
            {
                result.AddTag(tag);
            }
        }
        return result;
    }

    GameplayTagContainer Filter(const TEString &queryString) const { return Filter(GameplayTag(queryString)); }

    TEArray<GameplayTag> GetMatchingTags(const GameplayTag &queryTag) const
    {
        TEArray<GameplayTag> result;
        for (const auto &tag : m_Tags)
        {
            if (tag.MatchesTag(queryTag))
            {
                result.Add(tag);
            }
        }
        return result;
    }

    bool HasAllExact(const GameplayTagContainer &other) const
    {
        for (const auto &tag : other.m_Tags)
        {
            if (!HasTagExact(tag))
                return false;
        }
        return true;
    }

    bool HasAnyExact(const GameplayTagContainer &other) const
    {
        for (const auto &tag : other.m_Tags)
        {
            if (HasTagExact(tag))
                return true;
        }
        return false;
    }

    bool operator==(const GameplayTagContainer &other) const { return m_Tags == other.m_Tags; }
    bool operator!=(const GameplayTagContainer &other) const { return m_Tags != other.m_Tags; }

    TEArray<GameplayTag> GetTags() const
    {
        TEArray<GameplayTag> result;
        result.Reserve(static_cast<int32_t>(m_Tags.size()));
        for (const auto &t : m_Tags)
        {
            result.Add(t);
        }
        return result;
    }

    TEString GetTagsAsString() const
    {
        TEString result;
        bool first = true;
        for (const auto &tag : m_Tags)
        {
            if (!first)
                result += ", ";
            result += tag.ToString();
            first = false;
        }
        return result;
    }

private:
    TESet<GameplayTag> m_Tags;
};
