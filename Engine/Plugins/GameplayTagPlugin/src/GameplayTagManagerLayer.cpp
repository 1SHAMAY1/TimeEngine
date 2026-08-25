#include "GameplayTagManagerLayer.hpp"
#include "Core/Log.h"
#include "GameplayTagWidgets.hpp"
#include <algorithm>
#include <cstring>


GameplayTagManagerLayer::GameplayTagManagerLayer(const TEString &name)
    : Layer(name)
{
    m_SearchBar = CreateRef<UISearchBar>("Search gameplay tags...", "##GameplayTagManagerSearch");
}

GameplayTagManagerLayer::~GameplayTagManagerLayer() {}

void GameplayTagManagerLayer::OnAttach()
{
    TE_CORE_INFO("[GameplayTagManagerLayer] Attached.");
    RefreshDiagnostics();
}

void GameplayTagManagerLayer::OnDetach()
{
    TE_CORE_INFO("[GameplayTagManagerLayer] Detached.");
}

void GameplayTagManagerLayer::OnUpdate()
{
    if (m_StatusTimer > 0.0f)
    {
        m_StatusTimer -= 0.016f; // approximate frame delta
        if (m_StatusTimer <= 0.0f)
        {
            m_StatusMessage.Clear();
        }
    }
}

void GameplayTagManagerLayer::OnTimeGUIRender()
{
    if (!m_IsVisible)
        return;

    RenderMainWindow();
}

void GameplayTagManagerLayer::OnEvent(Event &event)
{
}

void GameplayTagManagerLayer::RefreshDiagnostics()
{
    m_CachedIssues = GameplayTagManager::Get().ValidateTags();
}

void GameplayTagManagerLayer::RenderMainWindow()
{
    TimeGUI::SetNextWindowPos(m_WindowPos, TimeGUICond_FirstUseEver);
    TimeGUI::SetNextWindowSize(m_WindowSize, TimeGUICond_FirstUseEver);

    if (!TimeGUI::Begin(m_WindowTitle.c_str(), &m_IsVisible))
    {
        TimeGUI::End();
        return;
    }

    // Top Header & Stats Bar
    size_t totalTags = GameplayTagManager::Get().GetTagCount();
    TimeGUI::Text("Registered Tags: %zu", totalTags);
    TimeGUI::SameLine();
    TimeGUI::Text(" | Issues: %zu", m_CachedIssues.size());

    if (!m_StatusMessage.empty())
    {
        TimeGUI::SameLine();
        TimeGUI::TextColored(TEVector4(0.2f, 0.9f, 0.4f, 1.0f), " | %s", m_StatusMessage.c_str());
    }

    TimeGUI::Separator();

    // 1. Search Bar Widget
    if (m_SearchBar)
    {
        m_SearchBar->Draw();
        TimeGUI::Spacing();
    }

    // 2. Main Content Split: Left (Tag Tree & Flat List), Right (Details / Editor)
    float windowWidth = m_WindowSize.x > 0.0f ? m_WindowSize.x : 700.0f;
    float leftColWidth = windowWidth * 0.50f;

    TimeGUI::BeginChild("TagHierarchyPanel", TEVector2(leftColWidth, 260), true);
    RenderTagHierarchyTree();
    TimeGUI::EndChild();

    TimeGUI::SameLine();

    TimeGUI::BeginChild("TagDetailsPanel", TEVector2(0, 260), true);
    RenderSelectedTagDetails();
    TimeGUI::EndChild();

    TimeGUI::Separator();

    // 3. Tab bar or sections: Add New Tag | INI File Management | Diagnostics & Auto-Fix
    if (TimeGUI::CollapsingHeader("➕ Add New Gameplay Tag", TimeGUITreeNodeFlags_DefaultOpen))
    {
        RenderAddNewTagSection();
    }

    if (TimeGUI::CollapsingHeader("📁 INI Configuration File (Load / Save)"))
    {
        RenderINISection();
    }

    if (TimeGUI::CollapsingHeader("🛠️ Tag Diagnostics & Auto-Fix"))
    {
        RenderDiagnosticsSection();
    }

    TimeGUI::End();
}

static void RenderTagTreeNodeInManager(const GameplayTagTreeNode &node, GameplayTag &selectedTag,
                                       const TERef<UISearchBar> &searchBar)
{
    for (const auto &pair : node.Children)
    {
        const GameplayTagTreeNode &child = pair.second;
        GameplayTag tag(child.FullTagPath);

        if (searchBar && !searchBar->GetQuery().empty())
        {
            if (!searchBar->Matches(child.FullTagPath) && !searchBar->Matches(child.SegmentName))
            {
                // Continue recursively if subchildren might match
                RenderTagTreeNodeInManager(child, selectedTag, searchBar);
                continue;
            }
        }

        bool hasSubChildren = !child.Children.empty();
        bool isSelected = (selectedTag == tag);

        TimeGUI::PushID(child.FullTagPath.c_str());

        if (hasSubChildren)
        {
            bool isOpen = TimeGUI::TreeNodeEx(child.SegmentName, TimeGUITreeNodeFlags_DefaultOpen);
            TimeGUI::SameLine();
            if (TimeGUI::SmallButton("Select"))
            {
                selectedTag = tag;
            }

            if (isOpen)
            {
                RenderTagTreeNodeInManager(child, selectedTag, searchBar);
                TimeGUI::TreePop();
            }
        }
        else
        {
            if (TimeGUI::Selectable(child.SegmentName.c_str(), isSelected))
            {
                selectedTag = tag;
            }
        }

        TimeGUI::PopID();
    }
}

void GameplayTagManagerLayer::RenderTagHierarchyTree()
{
    TimeGUI::Text("Tag Hierarchy");
    TimeGUI::Separator();

    auto tags = GameplayTagManager::Get().GetRegisteredTags();
    if (tags.empty())
    {
        TimeGUI::TextDisabled("No gameplay tags registered.");
        return;
    }

    GameplayTagTreeNode tree = BuildTagHierarchyTree(tags);
    RenderTagTreeNodeInManager(tree, m_SelectedTag, m_SearchBar);
}

void GameplayTagManagerLayer::RenderSelectedTagDetails()
{
    TimeGUI::Text("Tag Inspector");
    TimeGUI::Separator();

    if (!m_SelectedTag.IsValid())
    {
        TimeGUI::TextDisabled("Select a tag from the hierarchy tree to inspect and edit.");
        return;
    }

    TimeGUI::Text("Tag Name: %s", m_SelectedTag.ToString().c_str());
    TimeGUI::Text("Root Tag: %s", m_SelectedTag.GetRootName().c_str());
    TimeGUI::Text("Leaf Name: %s", m_SelectedTag.GetLeafName().c_str());
    TimeGUI::Text("Depth: %zu", m_SelectedTag.GetDepth());

    bool isNative = GameplayTagManager::Get().IsNativeTag(m_SelectedTag);
    if (isNative)
    {
        TimeGUI::TextColored(TEVector4(0.3f, 0.7f, 1.0f, 1.0f), "Source: Native (C++ Code)");
    }
    else
    {
        TimeGUI::TextColored(TEVector4(0.8f, 0.8f, 0.8f, 1.0f), "Source: Dynamic / INI");
    }

    TimeGUI::Spacing();
    TimeGUI::Separator();
    TimeGUI::Text("Description:");

    TEString currentDesc = GameplayTagManager::Get().GetTagDescription(m_SelectedTag);
    if (m_DescEditBuffer.IsEmpty() && !currentDesc.empty())
    {
        m_DescEditBuffer = currentDesc;
    }

    TimeGUI::InputText("##DescEdit", m_DescEditBuffer);
    if (TimeGUI::Button("Update Description"))
    {
        GameplayTagManager::Get().SetTagDescription(m_SelectedTag, m_DescEditBuffer);
        m_StatusMessage = "Updated description for " + m_SelectedTag.ToString();
        m_StatusTimer = 3.0f;
    }

    TimeGUI::Spacing();
    TimeGUI::Separator();
    TimeGUI::Text("Rename Tag:");

    TimeGUI::InputText("New Name##RenameInput", m_RenameBuffer);
    TimeGUI::SameLine();
    if (TimeGUI::Button("Rename"))
    {
        TEString newName = m_RenameBuffer;
        if (!newName.empty())
        {
            if (GameplayTagManager::Get().RenameTag(m_SelectedTag, newName))
            {
                m_StatusMessage = "Renamed " + m_SelectedTag.ToString() + " -> " + newName;
                m_SelectedTag = GameplayTag(newName);
                m_RenameBuffer.Clear();
                m_DescEditBuffer.Clear();
                m_StatusTimer = 3.0f;
                RefreshDiagnostics();
            }
        }
    }

    TimeGUI::Spacing();
    TimeGUI::Separator();

    if (TimeGUI::Button("🗑️ Delete Tag", TEVector2(120, 0)))
    {
        TEString deletedName = m_SelectedTag.ToString();
        GameplayTagManager::Get().RemoveTag(m_SelectedTag);
        m_SelectedTag = GameplayTag();
        m_RenameBuffer.Clear();
        m_DescEditBuffer.Clear();
        m_StatusMessage = "Deleted tag: " + deletedName;
        m_StatusTimer = 3.0f;
        RefreshDiagnostics();
    }
}

void GameplayTagManagerLayer::RenderAddNewTagSection()
{
    TimeGUI::InputText("Tag Name (e.g. Character.Enemy.Boss)##NewTagInput", m_NewTagNameBuffer);
    TimeGUI::InputText("Description (Optional)##NewTagDescInput", m_NewTagDescBuffer);

    if (TimeGUI::Button("Add Tag to Registry", TEVector2(160, 0)))
    {
        TEString newTagStr = m_NewTagNameBuffer;
        if (!newTagStr.empty())
        {
            GameplayTag registered = GameplayTagManager::Get().RegisterTag(newTagStr, m_NewTagDescBuffer);
            m_SelectedTag = registered;
            m_StatusMessage = "Added tag: " + registered.ToString();
            m_StatusTimer = 3.0f;
            m_NewTagNameBuffer.Clear();
            m_NewTagDescBuffer.Clear();
            RefreshDiagnostics();
        }
    }
}

void GameplayTagManagerLayer::RenderINISection()
{
    TimeGUI::InputText("INI File Path##INIPathInput", m_INIFilePathBuffer);

    if (TimeGUI::Button("📥 Load / Import from INI", TEVector2(200, 0)))
    {
        if (GameplayTagManager::Get().LoadTagsFromINI(m_INIFilePathBuffer))
        {
            m_StatusMessage = "Successfully imported tags from " + TEString(m_INIFilePathBuffer);
            RefreshDiagnostics();
        }
        else
        {
            m_StatusMessage = "Failed to open INI file: " + TEString(m_INIFilePathBuffer);
        }
        m_StatusTimer = 4.0f;
    }

    TimeGUI::SameLine();

    if (TimeGUI::Button("💾 Save / Export to INI", TEVector2(200, 0)))
    {
        if (GameplayTagManager::Get().SaveTagsToINI(m_INIFilePathBuffer))
        {
            m_StatusMessage = "Successfully saved tags to " + TEString(m_INIFilePathBuffer);
        }
        else
        {
            m_StatusMessage = "Failed to write INI file: " + TEString(m_INIFilePathBuffer);
        }
        m_StatusTimer = 4.0f;
    }
}

void GameplayTagManagerLayer::RenderDiagnosticsSection()
{
    if (TimeGUI::Button("🔄 Refresh Diagnostics"))
    {
        RefreshDiagnostics();
    }

    TimeGUI::SameLine();

    if (TimeGUI::Button("🔧 Auto-Fix All Tags", TEVector2(160, 0)))
    {
        size_t fixed = GameplayTagManager::Get().FixTags();
        m_StatusMessage = "Fixed " + TEString::FromInt64(static_cast<int64_t>(fixed)) + " tag issue(s).";
        m_StatusTimer = 4.0f;
        RefreshDiagnostics();
    }

    TimeGUI::Spacing();

    if (m_CachedIssues.IsEmpty())
    {
        TimeGUI::TextColored(TEVector4(0.2f, 0.9f, 0.4f, 1.0f), "✅ All registered gameplay tags are valid! No issues found.");
    }
    else
    {
        TimeGUI::TextColored(TEVector4(1.0f, 0.6f, 0.2f, 1.0f), "Found %zu issue(s):", static_cast<size_t>(m_CachedIssues.Num()));
        for (int32_t i = 0; i < m_CachedIssues.Num(); ++i)
        {
            const auto &issue = m_CachedIssues[i];
            if (issue.IssueSeverity == TagValidationIssue::Severity::Error)
            {
                TimeGUI::TextColored(TEVector4(1.0f, 0.3f, 0.3f, 1.0f), "  [ERROR] %s", issue.Message.c_str());
            }
            else
            {
                TimeGUI::TextColored(TEVector4(1.0f, 0.8f, 0.2f, 1.0f), "  [WARN]  %s", issue.Message.c_str());
            }
        }
    }
}

