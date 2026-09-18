#pragma once

#include <algorithm>
#include <cctype>
#include <functional>

class GameplayTag
{
public:
    GameplayTag() = default;
    explicit GameplayTag(const TEString &tagString) : m_Tag(SanitizeTag(tagString)) {}

    bool IsValid() const { return !m_Tag.empty(); }
    const TEString &ToString() const { return m_Tag; }
    const TEString &GetTagName() const { return m_Tag; }

    // Hierarchical match: Returns true if this tag matches or is a descendant of queryTag
    // Example: "Character.Enemy.Boss.Dragon".MatchesTag("Character.Enemy") -> true
    bool MatchesTag(const GameplayTag &queryTag) const
    {
        if (!IsValid() || !queryTag.IsValid())
            return false;

        const TEString &queryStr = queryTag.ToString();
        if (m_Tag == queryStr)
            return true;

        if (m_Tag.Len() > queryStr.Len())
        {
            if (m_Tag.StartsWith(queryStr))
            {
                return m_Tag[queryStr.Len()] == '.';
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
        if (dotPos == TEString::npos)
            return GameplayTag();

        return GameplayTag(m_Tag.substr(0, dotPos));
    }

    // Returns the leaf (last) segment of this tag, e.g. "Dragon" for "Character.Enemy.Boss.Dragon"
    TEString GetLeafName() const
    {
        if (!IsValid())
            return "";
        size_t dotPos = m_Tag.rfind('.');
        if (dotPos == TEString::npos)
            return m_Tag;
        return m_Tag.substr(dotPos + 1);
    }

    TEString GetTagNameOnly() const { return GetLeafName(); }

    // Returns the root (first) segment of this tag, e.g. "Character" for "Character.Enemy.Boss.Dragon"
    GameplayTag GetRootTag() const
    {
        if (!IsValid())
            return GameplayTag();
        size_t dotPos = m_Tag.find('.');
        if (dotPos == TEString::npos)
            return *this;
        return GameplayTag(m_Tag.substr(0, dotPos));
    }

    TEString GetRootName() const { return GetRootTag().ToString(); }

    // Creates a child tag by appending a sub-tag name, e.g. "Character.Enemy" + "Boss" -> "Character.Enemy.Boss"
    GameplayTag GetChildTag(const TEString &childSegment) const
    {
        if (!IsValid())
            return GameplayTag(childSegment);
        TEString sanitizedChild = SanitizeTag(childSegment);
        if (sanitizedChild.empty())
            return *this;
        return GameplayTag(m_Tag + "." + sanitizedChild);
    }

    // Number of hierarchical segments (depth)
    size_t GetDepth() const
    {
        if (!IsValid())
            return 0;
        size_t count = 1;
        for (char c : m_Tag)
        {
            if (c == '.')
                count++;
        }
        return count;
    }

    size_t GetSegmentCount() const { return GetDepth(); }

    // Returns segment at 0-based index
    TEString GetSegment(size_t index) const
    {
        if (!IsValid())
            return "";
        size_t currentIdx = 0;
        size_t start = 0;
        for (size_t i = 0; i <= m_Tag.length(); ++i)
        {
            if (i == m_Tag.length() || m_Tag[i] == '.')
            {
                if (currentIdx == index)
                {
                    return m_Tag.substr(start, i - start);
                }
                currentIdx++;
                start = i + 1;
            }
        }
        return "";
    }

    bool IsDirectParentOf(const GameplayTag &other) const
    {
        if (!IsValid() || !other.IsValid())
            return false;
        return other.GetParentTag() == *this;
    }

    bool IsChildOf(const GameplayTag &other) const { return MatchesTag(other) && *this != other; }

    bool operator==(const GameplayTag &other) const { return m_Tag == other.m_Tag; }
    bool operator!=(const GameplayTag &other) const { return m_Tag != other.m_Tag; }
    bool operator<(const GameplayTag &other) const { return m_Tag < other.m_Tag; }
    bool operator<=(const GameplayTag &other) const { return m_Tag <= other.m_Tag; }
    bool operator>(const GameplayTag &other) const { return m_Tag > other.m_Tag; }
    bool operator>=(const GameplayTag &other) const { return m_Tag >= other.m_Tag; }

private:
    static TEString SanitizeTag(const TEString &rawTag)
    {
        TEString result = rawTag;
        // Trim leading and trailing whitespace / dots
        size_t start = result.find_first_not_of(" \t\n\r.");
        if (start == TEString::npos)
            return "";
        size_t end = result.find_last_not_of(" \t\n\r.");
        return result.substr(start, end - start + 1);
    }

private:
    TEString m_Tag;
};

namespace std
{
template <> struct hash<GameplayTag>
{
    size_t operator()(const GameplayTag &tag) const noexcept { return std::hash<TEString>{}(tag.ToString()); }
};
} // namespace std
