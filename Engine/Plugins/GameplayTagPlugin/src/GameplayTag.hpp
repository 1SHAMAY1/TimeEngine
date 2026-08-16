#pragma once

#include <algorithm>
#include <cctype>
#include <functional>
#include <string>
#include <vector>

namespace TE
{

class GameplayTag
{
public:
    GameplayTag() = default;
    explicit GameplayTag(const std::string &tagString) : m_Tag(SanitizeTag(tagString)) {}

    bool IsValid() const { return !m_Tag.empty(); }
    const std::string &ToString() const { return m_Tag; }
    const std::string &GetTagName() const { return m_Tag; }

    // Hierarchical match: Returns true if this tag matches or is a descendant of queryTag
    // Example: "Character.Enemy.Boss.Dragon".MatchesTag("Character.Enemy") -> true
    bool MatchesTag(const GameplayTag &queryTag) const
    {
        if (!IsValid() || !queryTag.IsValid())
            return false;

        const std::string &queryStr = queryTag.ToString();
        if (m_Tag == queryStr)
            return true;

        if (m_Tag.length() > queryStr.length())
        {
            if (m_Tag.compare(0, queryStr.length(), queryStr) == 0)
            {
                return m_Tag[queryStr.length()] == '.';
            }
        }

        return false;
    }

    bool MatchesExact(const GameplayTag &other) const { return m_Tag == other.m_Tag; }

    GameplayTag GetParentTag() const
    {
        if (!IsValid())
            return GameplayTag();

        size_t dotPos = m_Tag.rfind('.');
        if (dotPos == std::string::npos)
            return GameplayTag();

        return GameplayTag(m_Tag.substr(0, dotPos));
    }

    bool operator==(const GameplayTag &other) const { return m_Tag == other.m_Tag; }
    bool operator!=(const GameplayTag &other) const { return m_Tag != other.m_Tag; }
    bool operator<(const GameplayTag &other) const { return m_Tag < other.m_Tag; }

private:
    static std::string SanitizeTag(const std::string &rawTag)
    {
        std::string result = rawTag;
        // Trim leading and trailing whitespace / dots
        size_t start = result.find_first_not_of(" \t\n\r.");
        if (start == std::string::npos)
            return "";
        size_t end = result.find_last_not_of(" \t\n\r.");
        return result.substr(start, end - start + 1);
    }

private:
    std::string m_Tag;
};

} // namespace TE

namespace std
{
template <> struct hash<TE::GameplayTag>
{
    size_t operator()(const TE::GameplayTag &tag) const noexcept { return std::hash<std::string>{}(tag.ToString()); }
};
} // namespace std
