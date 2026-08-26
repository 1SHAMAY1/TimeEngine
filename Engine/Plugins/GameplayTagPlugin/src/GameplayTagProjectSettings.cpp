#include "GameplayTagProjectSettings.hpp"
#include "Core/PreRequisites.h"
#include "GameplayTagManager.hpp"
#include "GameplayTagWidgets.hpp"
#include "Layers/EditorLayer.hpp"
#include "Utils/TimeGUI.hpp"

TE_REGISTER_PROJECT_SETTINGS(GameplayTagProjectSettings);

namespace
{
static TEString s_NewTagBuffer = "";
static TEString s_NewDescBuffer = "";
static TEString s_SearchFilter = "";
static bool s_ShowAddTagModal = false;
static TEString s_SelectedTagPath = "";

void DrawTagTreeRecursive(const GameplayTagTreeNode &node)
{
    for (const auto &pair : node.Children)
    {
        const auto &child = pair.second;
        bool isLeaf = child.Children.empty();
        TimeGUITreeNodeFlags flags = TimeGUITreeNodeFlags_OpenOnArrow;
        if (isLeaf)
            flags |= TimeGUITreeNodeFlags_Leaf;
        if (s_SelectedTagPath == child.FullTagPath)
            flags |= TimeGUITreeNodeFlags_Selected;

        bool open = TimeGUI::TreeNodeEx(child.SegmentName + "##" + child.FullTagPath, flags);
        if (TimeGUI::IsItemClicked())
        {
            s_SelectedTagPath = child.FullTagPath;
        }

        if (open)
        {
            DrawTagTreeRecursive(child);
            TimeGUI::TreePop();
        }
    }
}
} // namespace

void GameplayTagProjectSettings::OnDrawSettingsUI(Ref<EditorLayer> editor)
{
    auto &manager = GameplayTagManager::Get();

    TimeGUI::TextColored(TEColor(0.2f, 0.7f, 1.0f, 1.0f), "Gameplay Tags Manager");
    TimeGUI::Separator();

    // Action Row
    TimeGUI::InputTextWithHint("##SearchTags", "Search tags...", s_SearchFilter);
    TimeGUI::SameLine();
    if (TimeGUI::Button("Add New Tag", 120.0f, 0.0f))
    {
        s_ShowAddTagModal = true;
    }
    TimeGUI::SameLine();
    if (TimeGUI::Button("Save Tag Table", 130.0f, 0.0f))
    {
        manager.SaveTagsToINI("Config/GameplayTags.ini");
    }

    TimeGUI::Spacing();

    // Inline Add Tag Form
    if (s_ShowAddTagModal)
    {
        TimeGUI::PushStyleColor(TimeGUICol_ChildBg, TEColor(0.12f, 0.15f, 0.2f, 1.0f));
        TimeGUI::BeginChild("AddTagBox", TEVector2(0, 110), true);
        TimeGUI::TextColored(TEColor(0.3f, 0.8f, 0.4f, 1.0f), "Add New Tag Definition");
        TimeGUI::InputTextWithHint("Tag Path##NewTag", "e.g. Player.Status.Stunned", s_NewTagBuffer);
        TimeGUI::InputTextWithHint("Description##NewDesc", "Optional tag documentation", s_NewDescBuffer);
        if (TimeGUI::Button("Confirm Add", 110.0f, 0.0f))
        {
            TEString tagStr = s_NewTagBuffer.Trim();
            if (!tagStr.empty())
            {
                manager.RegisterTag(tagStr, s_NewDescBuffer);
                s_NewTagBuffer.Clear();
                s_NewDescBuffer.Clear();
                s_ShowAddTagModal = false;
            }
        }
        TimeGUI::SameLine();
        if (TimeGUI::Button("Cancel##AddTag", 80.0f, 0.0f))
        {
            s_ShowAddTagModal = false;
        }
        TimeGUI::EndChild();
        TimeGUI::PopStyleColor();
        TimeGUI::Spacing();
    }

    // Split View: Tag Hierarchy on Left, Tag Details on Right
    TimeGUI::BeginChild("TagTreeRegion", TEVector2(TimeGUI::GetContentRegionAvail().x * 0.55f, 280.0f), true);
    TimeGUI::TextDisabled("Registered Tag Hierarchy:");
    TimeGUI::Separator();

    auto registeredTags = manager.GetRegisteredTags();
    TEArray<GameplayTag> filteredTags;
    TEString filterStr = TEString(s_SearchFilter).ToLower();

    for (const auto &tag : registeredTags)
    {
        if (filterStr.empty() || tag.ToString().ToLower().Find(filterStr) != -1)
        {
            filteredTags.Add(tag);
        }
    }

    GameplayTagTreeNode tree = BuildTagHierarchyTree(filteredTags);
    DrawTagTreeRecursive(tree);
    TimeGUI::EndChild();

    TimeGUI::SameLine();

    // Details Panel on Right
    TimeGUI::BeginChild("TagDetailsRegion", TEVector2(0, 280.0f), true);
    TimeGUI::TextDisabled("Selected Tag Details:");
    TimeGUI::Separator();

    if (!s_SelectedTagPath.empty())
    {
        GameplayTag selTag(s_SelectedTagPath);
        TimeGUI::TextColored(TEColor(0.3f, 0.75f, 1.0f, 1.0f), "%s", s_SelectedTagPath.c_str());
        TEString desc = manager.GetTagDescription(selTag);
        TimeGUI::TextWrapped("Description: %s", desc.empty() ? "(No description)" : desc.c_str());
        TimeGUI::Text("Native Tag: %s", manager.IsNativeTag(selTag) ? "Yes" : "No");

        TimeGUI::Spacing();
        if (TimeGUI::Button("Delete Tag", 100.0f, 0.0f))
        {
            manager.RemoveTag(selTag);
            s_SelectedTagPath = "";
        }
    }
    else
    {
        TimeGUI::TextDisabled("Select a tag in the tree to view properties.");
    }
    TimeGUI::EndChild();

    TimeGUI::Spacing();
    TimeGUI::Separator();

    // Diagnostics / Validation
    auto issues = manager.ValidateTags();
    if (!issues.empty())
    {
        TimeGUI::TextColored(TEColor(1.0f, 0.4f, 0.4f, 1.0f), "Validation Issues (%u):", (uint32_t)issues.size());
        for (const auto &issue : issues)
        {
            TimeGUI::Text("  - [%s] %s: %s", issue.Tag.ToString().c_str(),
                          issue.IssueSeverity == TagValidationIssue::Severity::Error ? "ERROR" : "WARNING",
                          issue.Message.c_str());
        }
    }
    else
    {
        TimeGUI::TextColored(TEColor(0.4f, 0.85f, 0.4f, 1.0f), "[OK] All gameplay tags are valid and synchronized.");
    }
}
