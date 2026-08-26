#pragma once

#include "Core/Log.h"
#include "Core/PreRequisites.h"
#include "GameplayTag.hpp"
#include "GameplayTagContainer.hpp"
#include "Utils/TEFileSystem.hpp"
#include <algorithm>
#include <fstream>
#include <mutex>

struct TagValidationIssue
{
    enum class Severity
    {
        Warning,
        Error
    };

    GameplayTag Tag;
    TEString Message;
    Severity IssueSeverity = Severity::Warning;
};

class GameplayTagManager
{
public:
    static GameplayTagManager &Get()
    {
        static GameplayTagManager instance;
        return instance;
    }

    GameplayTag RegisterTag(const TEString &tagString, const TEString &description = "")
    {
        std::lock_guard<std::mutex> lock(m_Mutex);
        GameplayTag tag(tagString);
        if (tag.IsValid())
        {
            if (m_RegisteredTags.find(tag) == m_RegisteredTags.end() || !description.empty())
            {
                m_RegisteredTags[tag] = description;
                TE_CORE_INFO("[GameplayTagManager] Registered tag: '{0}' (Description: '{1}')", tagString, description);
            }

            // Register ancestor parent tags implicitly if not present
            GameplayTag parent = tag.GetParentTag();
            while (parent.IsValid())
            {
                if (m_RegisteredTags.find(parent) == m_RegisteredTags.end())
                {
                    m_RegisteredTags[parent] = "Implicit Parent Tag";
                    TE_CORE_INFO("[GameplayTagManager] Implicitly registered ancestor tag: '{0}'", parent.ToString());
                }
                parent = parent.GetParentTag();
            }
        }
        else
        {
            TE_CORE_WARN("[GameplayTagManager] Attempted to register invalid tag: '{0}'", tagString);
        }
        return tag;
    }

    GameplayTag RegisterNativeTag(const TEString &tagString, const TEString &description = "")
    {
        GameplayTag tag = RegisterTag(tagString, description);
        if (tag.IsValid())
        {
            std::lock_guard<std::mutex> lock(m_Mutex);
            m_NativeTags.insert(tag);
            TE_CORE_INFO("[GameplayTagManager] Registered native gameplay tag: '{0}'", tagString);
        }
        return tag;
    }

    bool IsTagRegistered(const GameplayTag &tag) const
    {
        std::lock_guard<std::mutex> lock(m_Mutex);
        return m_RegisteredTags.find(tag) != m_RegisteredTags.end();
    }

    bool IsTagRegistered(const TEString &tagString) const { return IsTagRegistered(GameplayTag(tagString)); }

    bool IsNativeTag(const GameplayTag &tag) const
    {
        std::lock_guard<std::mutex> lock(m_Mutex);
        return m_NativeTags.find(tag) != m_NativeTags.end();
    }

    bool RemoveTag(const GameplayTag &tag)
    {
        std::lock_guard<std::mutex> lock(m_Mutex);
        auto it = m_RegisteredTags.find(tag);
        if (it != m_RegisteredTags.end())
        {
            TEString tagStr = tag.ToString();
            m_RegisteredTags.erase(it);
            m_NativeTags.erase(tag);
            TE_CORE_INFO("[GameplayTagManager] Removed tag: '{0}'", tagStr);
            return true;
        }
        return false;
    }

    bool RemoveTag(const TEString &tagString) { return RemoveTag(GameplayTag(tagString)); }

    bool RenameTag(const GameplayTag &oldTag, const TEString &newTagName)
    {
        std::lock_guard<std::mutex> lock(m_Mutex);
        auto it = m_RegisteredTags.find(oldTag);
        if (it == m_RegisteredTags.end())
            return false;

        TEString desc = it->second;
        bool isNative = m_NativeTags.find(oldTag) != m_NativeTags.end();
        TEString oldStr = oldTag.ToString();

        m_RegisteredTags.erase(it);
        m_NativeTags.erase(oldTag);

        GameplayTag newTag(newTagName);
        if (newTag.IsValid())
        {
            m_RegisteredTags[newTag] = desc;
            if (isNative)
            {
                m_NativeTags.insert(newTag);
            }

            // Register parents of new tag
            GameplayTag parent = newTag.GetParentTag();
            while (parent.IsValid())
            {
                if (m_RegisteredTags.find(parent) == m_RegisteredTags.end())
                {
                    m_RegisteredTags[parent] = "Implicit Parent Tag";
                }
                parent = parent.GetParentTag();
            }
            TE_CORE_INFO("[GameplayTagManager] Renamed tag '{0}' -> '{1}'", oldStr, newTagName);
            return true;
        }
        return false;
    }

    TEString GetTagDescription(const GameplayTag &tag) const
    {
        std::lock_guard<std::mutex> lock(m_Mutex);
        auto it = m_RegisteredTags.find(tag);
        if (it != m_RegisteredTags.end())
        {
            return it->second;
        }
        return "";
    }

    void SetTagDescription(const GameplayTag &tag, const TEString &description)
    {
        std::lock_guard<std::mutex> lock(m_Mutex);
        if (tag.IsValid())
        {
            m_RegisteredTags[tag] = description;
        }
    }

    TEArray<GameplayTag> GetRegisteredTags() const
    {
        std::lock_guard<std::mutex> lock(m_Mutex);
        TEArray<GameplayTag> tags;
        tags.Reserve(static_cast<int32_t>(m_RegisteredTags.size()));
        for (const auto &pair : m_RegisteredTags)
        {
            tags.Add(pair.first);
        }
        std::sort(tags.GetData(), tags.GetData() + tags.Num());
        return tags;
    }

    const TEMap<GameplayTag, TEString> &GetRegisteredTagsMap() const { return m_RegisteredTags; }

    size_t GetTagCount() const
    {
        std::lock_guard<std::mutex> lock(m_Mutex);
        return m_RegisteredTags.Num();
    }

    void Clear()
    {
        std::lock_guard<std::mutex> lock(m_Mutex);
        m_RegisteredTags.clear();
        m_NativeTags.clear();
    }

    // ===== INI File Management =====

    bool LoadTagsFromINI(const TEString &filePath)
    {
        if (!TEFileSystem::Exists(filePath))
            return false;

        bool inTagsSection = false;

        bool success = TEFileSystem::ForEachLine(
            filePath,
            [this, &inTagsSection](const TEString &rawLine)
            {
                TEString line = rawLine.Trim();

                // Ignore comments
                if (line.IsEmpty() || line.StartsWith(";") || line.StartsWith("#"))
                    return true;

                // Section headers
                if (line.StartsWith("[") && line.EndsWith("]"))
                {
                    TEString section = line.Mid(1, line.Length() - 2).ToLower();
                    inTagsSection = (section == "gameplaytags" || section == "tags" || section == "gameplaytag");
                    return true;
                }

                // Parse key-value or raw tags
                int64_t eqPos = line.Find("=");
                if (eqPos != -1)
                {
                    TEString key = line.Mid(0, eqPos).Trim();
                    TEString value = line.Mid(eqPos + 1).Trim();
                    TEString keyLower = key.ToLower();

                    if (keyLower == "tag" || keyLower == "gameplaytag")
                    {
                        if (value.StartsWith("(") && value.EndsWith(")"))
                        {
                            TEString inner = value.Mid(1, value.Length() - 2);
                            TEString tagStr, commentStr;

                            int64_t tagKeyPos = inner.Find("Tag=\"");
                            if (tagKeyPos != -1)
                            {
                                int64_t tagStart = tagKeyPos + 5;
                                int64_t tagEnd = inner.Find("\"", ESearchCase::CaseSensitive, ESearchDir::FromStart,
                                                            static_cast<int>(tagStart));
                                if (tagEnd != -1)
                                    tagStr = inner.Mid(tagStart, tagEnd - tagStart);
                            }

                            int64_t devKeyPos = inner.Find("DevComment=\"");
                            if (devKeyPos != -1)
                            {
                                int64_t devStart = devKeyPos + 12;
                                int64_t devEnd = inner.Find("\"", ESearchCase::CaseSensitive, ESearchDir::FromStart,
                                                            static_cast<int>(devStart));
                                if (devEnd != -1)
                                    commentStr = inner.Mid(devStart, devEnd - devStart);
                            }

                            if (!tagStr.IsEmpty())
                            {
                                RegisterTag(tagStr, commentStr);
                            }
                        }
                        else
                        {
                            int64_t commaPos = value.Find(",");
                            if (commaPos != -1)
                            {
                                TEString t = value.Mid(0, commaPos).Trim();
                                TEString c = value.Mid(commaPos + 1).Trim();
                                RegisterTag(t, c);
                            }
                            else
                            {
                                RegisterTag(value);
                            }
                        }
                    }
                    else if (keyLower == "taglist")
                    {
                        TEArray<TEString> items = value.Split(',');
                        for (const auto &item : items)
                        {
                            TEString trimmedItem = item.Trim();
                            if (!trimmedItem.IsEmpty())
                            {
                                RegisterTag(trimmedItem);
                            }
                        }
                    }
                    else
                    {
                        RegisterTag(key, value);
                    }
                }
                else
                {
                    int64_t comma = line.Find(",");
                    if (comma != -1)
                    {
                        RegisterTag(line.Mid(0, comma).Trim(), line.Mid(comma + 1).Trim());
                    }
                    else
                    {
                        RegisterTag(line);
                    }
                }
                return true;
            });

        TE_CORE_INFO("[GameplayTagManager] Successfully loaded tags from INI file: '{0}' (Total registered: {1})",
                     filePath, m_RegisteredTags.Num());
        return success;
    }

    bool SaveTagsToINI(const TEString &filePath) const
    {
        if (filePath.HasParentPath())
        {
            TEFileSystem::CreateDirectories(filePath.GetParentPath());
        }

        std::ofstream file(filePath.c_str());
        if (!file.is_open())
        {
            TE_CORE_ERROR("[GameplayTagManager] Failed to open INI file for writing: '{0}'", filePath);
            return false;
        }

        std::lock_guard<std::mutex> lock(m_Mutex);

        file << "[GameplayTags]\n";
        file << "; TimeEngine Gameplay Tag Configuration\n\n";

        TEArray<GameplayTag> tags;
        for (const auto &pair : m_RegisteredTags)
        {
            tags.push_back(pair.first);
        }
        std::sort(tags.begin(), tags.end());

        for (const auto &tag : tags)
        {
            auto it = m_RegisteredTags.find(tag);
            TEString comment = (it != m_RegisteredTags.end()) ? it->second : "";

            if (!comment.IsEmpty())
            {
                file << "+Tag=(Tag=\"" << tag.ToString().c_str() << "\",DevComment=\"" << comment.c_str() << "\")\n";
            }
            else
            {
                file << "+Tag=(Tag=\"" << tag.ToString().c_str() << "\")\n";
            }
        }

        TE_CORE_INFO("[GameplayTagManager] Saved {0} tags to INI file: '{1}'", tags.Num(), filePath);
        return true;
    }

    // ===== Validation & Auto-Fix =====

    TEArray<TagValidationIssue> ValidateTags() const
    {
        std::lock_guard<std::mutex> lock(m_Mutex);
        TEArray<TagValidationIssue> issues;

        for (const auto &pair : m_RegisteredTags)
        {
            const GameplayTag &tag = pair.first;
            const TEString &tagStr = tag.ToString();

            if (tagStr.empty())
            {
                issues.Add({tag, "Empty tag name found.", TagValidationIssue::Severity::Error});
                continue;
            }

            // Check for consecutive dots
            if (tagStr.find("..") != TEString::npos)
            {
                issues.Add({tag, "Tag contains consecutive dots (empty segment): '" + tagStr + "'",
                            TagValidationIssue::Severity::Error});
            }

            // Check for invalid characters
            for (char c : tagStr)
            {
                if (!std::isalnum(static_cast<unsigned char>(c)) && c != '.' && c != '_')
                {
                    issues.Add({tag, TEString("Tag contains invalid character '") + c + "': '" + tagStr + "'",
                                TagValidationIssue::Severity::Warning});
                    break;
                }
            }

            // Check if ancestor tags are registered
            GameplayTag parent = tag.GetParentTag();
            while (parent.IsValid())
            {
                if (m_RegisteredTags.find(parent) == m_RegisteredTags.end())
                {
                    issues.Add({tag, "Missing ancestor parent tag '" + parent.ToString() + "' for tag '" + tagStr + "'",
                                TagValidationIssue::Severity::Warning});
                }
                parent = parent.GetParentTag();
            }
        }

        if (!issues.IsEmpty())
        {
            TE_CORE_WARN("[GameplayTagManager] ValidateTags found {0} validation issue(s).", issues.Num());
        }
        else
        {
            TE_CORE_INFO("[GameplayTagManager] ValidateTags found 0 issues. Tag hierarchy is clean.");
        }

        return issues;
    }

    size_t FixTags()
    {
        std::lock_guard<std::mutex> lock(m_Mutex);
        size_t fixCount = 0;

        TEMap<GameplayTag, TEString> oldMap = m_RegisteredTags;
        TESet<GameplayTag> oldNative = m_NativeTags;

        m_RegisteredTags.clear();
        m_NativeTags.clear();

        for (const auto &pair : oldMap)
        {
            TEString cleaned;
            bool lastWasDot = false;
            for (char c : pair.first.ToString())
            {
                if (std::isalnum(static_cast<unsigned char>(c)) || c == '_')
                {
                    cleaned += c;
                    lastWasDot = false;
                }
                else if (c == '.')
                {
                    if (!cleaned.empty() && !lastWasDot)
                    {
                        cleaned += '.';
                        lastWasDot = true;
                    }
                }
                else if (c == ' ' || c == '-' || c == ':')
                {
                    if (!cleaned.empty() && !lastWasDot)
                    {
                        cleaned += '.';
                        lastWasDot = true;
                    }
                }
            }

            // Strip trailing dot
            while (!cleaned.IsEmpty() && cleaned.EndsWith("."))
            {
                cleaned = cleaned.Left(cleaned.Len() - 1);
            }

            if (cleaned.IsEmpty())
            {
                fixCount++;
                continue;
            }

            if (cleaned != pair.first.ToString())
            {
                fixCount++;
            }

            GameplayTag newTag(cleaned);
            m_RegisteredTags[newTag] = pair.second;
            if (oldNative.find(pair.first) != oldNative.end())
            {
                m_NativeTags.insert(newTag);
            }

            // Ensure parent hierarchy is fully populated
            GameplayTag parent = newTag.GetParentTag();
            while (parent.IsValid())
            {
                if (m_RegisteredTags.find(parent) == m_RegisteredTags.end())
                {
                    m_RegisteredTags[parent] = "Implicit Parent Tag";
                    fixCount++;
                }
                parent = parent.GetParentTag();
            }
        }

        TE_CORE_INFO("[GameplayTagManager] FixTags completed. Repaired/resolved {0} issue(s).", fixCount);
        return fixCount;
    }

private:
    GameplayTagManager() = default;
    ~GameplayTagManager() = default;

    GameplayTagManager(const GameplayTagManager &) = delete;
    GameplayTagManager &operator=(const GameplayTagManager &) = delete;

private:
    mutable std::mutex m_Mutex;
    TEMap<GameplayTag, TEString> m_RegisteredTags;
    TESet<GameplayTag> m_NativeTags;
};

// ===== Native Gameplay Tag Macros =====

#define TE_DECLARE_GAMEPLAY_TAG(TagName) extern const GameplayTag TagName;

#define TE_DEFINE_GAMEPLAY_TAG_COMMENT(TagName, TagString, Comment)                                                    \
    const GameplayTag TagName = GameplayTagManager::Get().RegisterNativeTag(TagString, Comment);
