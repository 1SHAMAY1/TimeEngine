#pragma once

#include "GameplayTag.hpp"
#include "GameplayTagContainer.hpp"
#include <mutex>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace TE
{

class GameplayTagManager
{
public:
    static GameplayTagManager &Get()
    {
        static GameplayTagManager instance;
        return instance;
    }

    void RegisterTag(const std::string &tagString, const std::string &description = "")
    {
        std::lock_guard<std::mutex> lock(m_Mutex);
        GameplayTag tag(tagString);
        if (tag.IsValid())
        {
            m_RegisteredTags[tag] = description;
            // Also register ancestor parent tags implicitly if not present
            GameplayTag parent = tag.GetParentTag();
            while (parent.IsValid())
            {
                if (m_RegisteredTags.find(parent) == m_RegisteredTags.end())
                {
                    m_RegisteredTags[parent] = "Implicit Parent Tag";
                }
                parent = parent.GetParentTag();
            }
        }
    }

    bool IsTagRegistered(const GameplayTag &tag) const
    {
        std::lock_guard<std::mutex> lock(m_Mutex);
        return m_RegisteredTags.find(tag) != m_RegisteredTags.end();
    }

    bool IsTagRegistered(const std::string &tagString) const { return IsTagRegistered(GameplayTag(tagString)); }

    std::vector<GameplayTag> GetRegisteredTags() const
    {
        std::lock_guard<std::mutex> lock(m_Mutex);
        std::vector<GameplayTag> tags;
        tags.reserve(m_RegisteredTags.size());
        for (const auto &pair : m_RegisteredTags)
        {
            tags.push_back(pair.first);
        }
        return tags;
    }

    void Clear()
    {
        std::lock_guard<std::mutex> lock(m_Mutex);
        m_RegisteredTags.clear();
    }

private:
    GameplayTagManager() = default;
    ~GameplayTagManager() = default;

    GameplayTagManager(const GameplayTagManager &) = delete;
    GameplayTagManager &operator=(const GameplayTagManager &) = delete;

private:
    mutable std::mutex m_Mutex;
    std::unordered_map<GameplayTag, std::string> m_RegisteredTags;
};

} // namespace TE
