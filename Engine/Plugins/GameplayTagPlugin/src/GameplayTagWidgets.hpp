#pragma once
#include "Core/PreRequisites.h"
#include "Core/Scene/PropertyDrawers.hpp"
#include "GameplayTag.hpp"
#include "GameplayTagContainer.hpp"
#include "GameplayTagManager.hpp"
#include "Utils/TimeGUI.hpp"
#include <algorithm>

// Node structure for building hierarchical tree views of tags
struct GameplayTagTreeNode
{
    TEString SegmentName;
    TEString FullTagPath;
    bool IsRegisteredTag = false;
    TEMap<TEString, GameplayTagTreeNode> Children;
};

inline GameplayTagTreeNode BuildTagHierarchyTree(const TEArray<GameplayTag> &tags)
{
    GameplayTagTreeNode root;
    root.SegmentName = "Root";

    for (const auto &tag : tags)
    {
        const TEString &tagStr = tag.ToString();
        if (tagStr.IsEmpty())
            continue;

        GameplayTagTreeNode *curr = &root;
        TEArray<TEString> segments = tagStr.Split('.');
        TEString currentPath;

        for (const auto &segment : segments)
        {
            if (!currentPath.IsEmpty())
                currentPath += ".";
            currentPath += segment;

            if (curr->Children.find(segment) == curr->Children.end())
            {
                GameplayTagTreeNode node;
                node.SegmentName = segment;
                node.FullTagPath = currentPath;
                curr->Children[segment] = node;
            }
            curr = &curr->Children[segment];
        }
        curr->IsRegisteredTag = true;
    }

    return root;
}

// ===== Interactive Single GameplayTag Picker Widget =====

inline bool DrawGameplayTagWidget(const TEString &label, GameplayTag &tag)
{
    bool changed = false;
    TEString popupId = "GameplayTagPickerModal##" + label;

    TimeGUI::PushID(label.c_str());
    TimeGUI::Text("%s", label.c_str());
    TimeGUI::SameLine();

    TEString displayStr = tag.IsValid() ? tag.ToString() : "(None)";
    if (TimeGUI::Button(displayStr.c_str(), TEVector2(200, 0)))
    {
        TimeGUI::OpenPopup(popupId.c_str());
    }

    if (tag.IsValid())
    {
        TimeGUI::SameLine();
        if (TimeGUI::Button("x##ClearTag"))
        {
            tag = GameplayTag();
            changed = true;
        }
    }

    if (TimeGUI::BeginPopup(popupId.c_str()))
    {
        TimeGUI::Text("Select Gameplay Tag");
        TimeGUI::Separator();

        static TEString filterBuf;
        TimeGUI::InputText("Search##TagPicker", filterBuf);
        TEString filter = filterBuf.ToLower();

        TimeGUI::BeginChild("TagPickerTree", TEVector2(320, 220), true);

        auto allTags = GameplayTagManager::Get().GetRegisteredTags();
        if (allTags.IsEmpty())
        {
            TimeGUI::TextDisabled("No tags registered in manager.");
        }

        // Render matching tags or tree
        for (const auto &registeredTag : allTags)
        {
            const TEString &tagStr = registeredTag.ToString();
            TEString lowerStr = tagStr;
            std::transform(lowerStr.begin(), lowerStr.end(), lowerStr.begin(), ::tolower);

            if (!filter.empty() && lowerStr.find(filter) == TEString::npos)
                continue;

            bool isSelected = (tag == registeredTag);
            if (TimeGUI::Selectable(tagStr.c_str(), isSelected))
            {
                tag = registeredTag;
                changed = true;
                TimeGUI::CloseCurrentPopup();
            }
        }

        TimeGUI::EndChild();
        TimeGUI::Separator();

        if (TimeGUI::Button("Clear Selection", TEVector2(120, 0)))
        {
            tag = GameplayTag();
            changed = true;
            TimeGUI::CloseCurrentPopup();
        }
        TimeGUI::SameLine();
        if (TimeGUI::Button("Close", TEVector2(80, 0)))
        {
            TimeGUI::CloseCurrentPopup();
        }

        TimeGUI::EndPopup();
    }

    TimeGUI::PopID();
    return changed;
}

// Recursive helper to draw tag tree with checkboxes
inline void DrawTagTreeNodeCheckboxes(const GameplayTagTreeNode &node, GameplayTagContainer &container,
                                     const TEString &filter, bool &outChanged)
{
    for (const auto &pair : node.Children)
    {
        const GameplayTagTreeNode &child = pair.second;
        GameplayTag tag(child.FullTagPath);

        TEString lowerPath = child.FullTagPath;
        std::transform(lowerPath.begin(), lowerPath.end(), lowerPath.begin(), ::tolower);

        bool passesFilter = filter.empty() || lowerPath.find(filter) != TEString::npos;

        bool hasSubChildren = !child.Children.empty();
        TimeGUIWindowFlags treeFlags = TimeGUIWindowFlags_None;

        TimeGUI::PushID(child.FullTagPath.c_str());

        bool isChecked = container.HasTagExact(tag);
        if (TimeGUI::Checkbox("##chk", &isChecked))
        {
            if (isChecked)
            {
                container.AddTag(tag);
            }
            else
            {
                container.RemoveTag(tag);
            }
            outChanged = true;
        }

        TimeGUI::SameLine();

        if (hasSubChildren)
        {
            bool isOpen = TimeGUI::TreeNodeEx(child.SegmentName, TimeGUITreeNodeFlags_DefaultOpen);
            if (isOpen)
            {
                DrawTagTreeNodeCheckboxes(child, container, filter, outChanged);
                TimeGUI::TreePop();
            }
        }
        else
        {
            TimeGUI::Text("%s", child.SegmentName.c_str());
        }

        TimeGUI::PopID();
    }
}

// ===== Interactive GameplayTagContainer Multi-Select / Checkbox Tree Widget =====

inline bool DrawGameplayTagContainerWidget(const TEString &label, GameplayTagContainer &container)
{
    bool changed = false;
    TEString popupId = "GameplayTagContainerPickerModal##" + label;

    TimeGUI::PushID(label.c_str());
    TimeGUI::Text("%s (%zu tags)", label.c_str(), container.Size());

    // Render active tags as removable chips / badges
    auto currentTags = container.GetTags();
    if (currentTags.IsEmpty())
    {
        TimeGUI::TextDisabled("  (No tags assigned)");
    }
    else
    {
        for (const auto &tag : currentTags)
        {
            TEString chipText = tag.ToString() + "  x##Remove_" + tag.ToString();
            if (TimeGUI::SmallButton(chipText.c_str()))
            {
                container.RemoveTag(tag);
                changed = true;
            }
            TimeGUI::SameLine();
        }
        TimeGUI::NewLine();
    }

    if (TimeGUI::Button("Edit Tags / Checkboxes...", TEVector2(180, 0)))
    {
        TimeGUI::OpenPopup(popupId.c_str());
    }

    if (TimeGUI::BeginPopup(popupId.c_str()))
    {
        TimeGUI::Text("Gameplay Tag Container Editor");
        TimeGUI::Separator();

        static TEString filterBuf;
        TimeGUI::InputText("Search##ContainerFilter", filterBuf);
        TEString filter = filterBuf.ToLower();

        TimeGUI::BeginChild("TagContainerTree", TEVector2(380, 260), true);

        auto allTags = GameplayTagManager::Get().GetRegisteredTags();
        if (allTags.IsEmpty())
        {
            TimeGUI::TextDisabled("No tags registered in manager.");
        }
        else
        {
            GameplayTagTreeNode tree = BuildTagHierarchyTree(allTags);
            DrawTagTreeNodeCheckboxes(tree, container, filter, changed);
        }

        TimeGUI::EndChild();
        TimeGUI::Separator();

        // Quick add new tag
        static TEString newTagBuf;
        TimeGUI::InputText("New Tag##QuickAdd", newTagBuf);
        TimeGUI::SameLine();
        if (TimeGUI::Button("Add & Assign##QuickBtn"))
        {
            if (!newTagBuf.empty())
            {
                GameplayTag registered = GameplayTagManager::Get().RegisterTag(newTagBuf);
                container.AddTag(registered);
                changed = true;
                newTagBuf.Clear();
            }
        }

        TimeGUI::Separator();

        if (TimeGUI::Button("Clear All", TEVector2(100, 0)))
        {
            container.Clear();
            changed = true;
        }
        TimeGUI::SameLine();
        if (TimeGUI::Button("Close", TEVector2(100, 0)))
        {
            TimeGUI::CloseCurrentPopup();
        }

        TimeGUI::EndPopup();
    }

    TimeGUI::PopID();
    return changed;
}

// ===== TEPropertyDrawer Specializations =====

template <> struct TEPropertyDrawer<GameplayTag>
{
    static void Draw(void *addr, const TEString &displayName)
    {
        if (addr)
        {
            DrawGameplayTagWidget(displayName, *(GameplayTag *)addr);
        }
    }

    static TEString Serialize(void *addr)
    {
        if (!addr)
            return "";
        return ((GameplayTag *)addr)->ToString();
    }

    static void Deserialize(void *addr, const TEString &data)
    {
        if (addr)
        {
            *(GameplayTag *)addr = GameplayTag(data);
        }
    }
};

template <> struct TEPropertyDrawer<GameplayTagContainer>
{
    static void Draw(void *addr, const TEString &displayName)
    {
        if (addr)
        {
            DrawGameplayTagContainerWidget(displayName, *(GameplayTagContainer *)addr);
        }
    }

    static TEString Serialize(void *addr)
    {
        if (!addr)
            return "";
        return ((GameplayTagContainer *)addr)->GetTagsAsString();
    }

    static void Deserialize(void *addr, const TEString &data)
    {
        if (addr)
        {
            auto *container = (GameplayTagContainer *)addr;
            container->Clear();
            TEArray<TEString> tags = data.Split(',');
            for (const auto &tagStr : tags)
            {
                TEString trimmed = tagStr.Trim();
                if (!trimmed.IsEmpty())
                {
                    container->AddTag(trimmed);
                }
            }
        }
    }
};

