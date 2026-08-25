#include "Core/PreRequisites.h"
#include "Editor/Panels/PluginsPanel.hpp"
#include "Core/Plugin/PluginManager.hpp"
#include "Core/Project/Project.hpp"
#include "Layers/EditorLayer.hpp"
#include "UI/Widgets/UISearchBar.hpp"
#include "UI/Widgets/UIToggleSwitch.hpp"
#include "Editor/Panels/IEditorPanel.hpp"


PluginsPanel::PluginsPanel()
    : IEditorPanel("Plugins")
{
    m_Visible = false;
    m_SearchBar = CreateRef<UISearchBar>("Search installed plugins...", "##PluginsSearchBar");
}

void PluginsPanel::OnTimeGUIRender(Ref<EditorLayer> editor)
{
    if (!editor || !m_Visible)
        return;

    TimeGUI::SetNextWindowSize(TEVector2(720.0f, 520.0f), TimeGUICond_FirstUseEver);
    TimeGUI::Begin(GetTitle().c_str(), &m_Visible);

    // ── 1. Top Action Bar: Search Bar & Refresh Button ────────────────────────
    float availWidth = TimeGUI::GetContentRegionAvail().x;
    float refreshBtnWidth = 140.0f;
    float searchWidth = availWidth - refreshBtnWidth - 12.0f;

    if (m_SearchBar)
    {
        m_SearchBar->SetSize(TEVector2(searchWidth > 180.0f ? searchWidth : 180.0f, 0.0f));
        m_SearchBar->Draw();
    }

    TimeGUI::SameLine(0, 12.0f);
    TimeGUI::PushStyleColor(TimeGUICol_Button, TEVector4(0.18f, 0.22f, 0.30f, 0.95f));
    TimeGUI::PushStyleColor(TimeGUICol_ButtonHovered, TEVector4(0.24f, 0.32f, 0.44f, 1.0f));
    if (TimeGUI::Button("Refresh Plugins", TEVector2(refreshBtnWidth, 30.0f)))
    {
        PluginManager::DiscoverPlugins();
    }
    TimeGUI::PopStyleColor(2);

    TimeGUI::Spacing();
    TimeGUI::Separator();
    TimeGUI::Spacing();

    // ── 2. Discover Plugins Check ─────────────────────────────────────────────
    if (PluginManager::GetDiscoveredPlugins().empty())
    {
        PluginManager::DiscoverPlugins();
    }

    const auto &plugins = PluginManager::GetDiscoveredPlugins();
    if (plugins.empty())
    {
        TimeGUI::Spacing();
        TimeGUI::TextColored(TEVector4(0.6f, 0.65f, 0.75f, 1.0f), "No plugins discovered in engine or project directories.");
        TimeGUI::Spacing();
        if (TimeGUI::Button("Scan Again", TEVector2(120.0f, 28.0f)))
        {
            PluginManager::DiscoverPlugins();
        }
    }
    else
    {
        // ── 3. Scrollable List of Plugin Cards ─────────────────────────────────
        TimeGUI::BeginChild("##PluginsListRegion", TEVector2(0, 0), false, TimeGUIWindowFlags_AlwaysVerticalScrollbar);

        int visibleCount = 0;
        for (size_t i = 0; i < plugins.size(); ++i)
        {
            const auto &plugin = plugins[i];
            if (m_SearchBar && !m_SearchBar->Matches(plugin.Name) && !m_SearchBar->Matches(plugin.Description) && !m_SearchBar->Matches(plugin.Author))
                continue;

            visibleCount++;
            TimeGUI::PushID((int)i);

            // Card Container Layout Allocation with generous dimensions
            float cardWidth = TimeGUI::GetContentRegionAvail().x;
            float cardHeight = 96.0f;
            TimeGUI::Dummy(TEVector2(cardWidth, cardHeight));

            TEVector2 cardMin = TimeGUI::GetItemRectMin();
            TEVector2 cardMax = TimeGUI::GetItemRectMax();
            bool isHovered = TimeGUI::IsItemHovered();
            TimeGUIDrawList dl = TimeGUI::GetWindowDrawList();

            // Background & Border with hover highlight
            bool isEnabled = plugin.Enabled;
            unsigned int bgCol = isHovered ? (isEnabled ? 0xDD1B2433 : 0xBB161A22) : (isEnabled ? 0xBB141B26 : 0x8811151D);
            unsigned int borderCol = isHovered ? (isEnabled ? 0xFF4A88E8 : 0x88405068) : (isEnabled ? 0x66334D6E : 0x44242C38);
            dl.AddRectFilled(cardMin, cardMax, bgCol, 8.0f);
            dl.AddRect(cardMin, cardMax, borderCol, 8.0f, 0, 1.2f);

            // Left Thumbnail / Icon Box (64x64, vertically centered with 16px padding)
            float thumbSize = 64.0f;
            float thumbPad = (cardHeight - thumbSize) * 0.5f;
            TEVector2 thumbMin = TEVector2(cardMin.x + 16.0f, cardMin.y + thumbPad);
            TEVector2 thumbMax = TEVector2(thumbMin.x + thumbSize, thumbMin.y + thumbSize);

            auto instance = PluginManager::GetPluginInstance(plugin.Name);
            if (instance)
            {
                instance->DrawThumbnail(dl, thumbMin, thumbMax);
            }
            else
            {
                dl.AddRectFilled(thumbMin, thumbMax, 0xFF161E2A, 8.0f);
                dl.AddRect(thumbMin, thumbMax, 0xFF2B3A4F, 8.0f, 0, 1.0f);
                TEVector2 center = TEVector2(thumbMin.x + thumbSize * 0.5f, thumbMin.y + thumbSize * 0.5f);
                dl.AddCircleFilled(center, 14.0f, 0xFF24344A);
                dl.AddCircle(center, 14.0f, 0xFF446690, 16, 1.5f);
            }

            // Right Metadata Column (starts with comfortable 20px gap from thumbnail)
            float textStartX = thumbMax.x + 18.0f;

            // Row 1: Plugin Title & Version
            TEVector2 titlePos = TEVector2(textStartX, cardMin.y + 14.0f);
            dl.AddText(titlePos, 0xFFFFFFFF, plugin.Name.c_str());

            float titleWidth = TimeGUI::CalcTextSize(plugin.Name).x;
            TEString verText = "v" + (plugin.Version.empty() ? TEString("1.0.0") : plugin.Version);
            TEVector2 verPos = TEVector2(textStartX + titleWidth + 12.0f, cardMin.y + 15.0f);
            TEVector2 verSize = TimeGUI::CalcTextSize(verText);
            dl.AddRectFilled(TEVector2(verPos.x - 4.0f, verPos.y - 1.0f), TEVector2(verPos.x + verSize.x + 4.0f, verPos.y + verSize.y + 1.0f), 0x334477AA, 4.0f);
            dl.AddText(verPos, 0xFF9FC4E8, verText.c_str());

            // Row 2: Author
            TEString authorText = "Author: " + (plugin.Author.empty() ? TEString("TimeEngine Team") : plugin.Author);
            TEVector2 authorPos = TEVector2(textStartX, cardMin.y + 38.0f);
            dl.AddText(authorPos, 0xFF6D829A, authorText.c_str());

            // Row 3: Description (Truncated / Wrapped)
            TEString descText = plugin.Description.empty() ? "No description provided." : plugin.Description;
            if (descText.length() > 95)
                descText = descText.substr(0, 92) + "...";
            TEVector2 descPos = TEVector2(textStartX, cardMin.y + 60.0f);
            dl.AddText(descPos, 0xFFA0AFBE, descText.c_str());

            // Phone-Style Pill Toggle Switch on Right of Card (vertically centered)
            float switchWidth = 52.0f;
            float switchHeight = 28.0f;
            float switchY = cardMin.y + (cardHeight - switchHeight) * 0.5f;
            TimeGUI::SetCursorScreenPos(TEVector2(cardMax.x - switchWidth - 22.0f, switchY));

            bool switchState = isEnabled;
            if (UIToggleSwitch::DrawToggle("##PluginToggle", switchState, TEVector2(switchWidth, switchHeight)))
            {
                PluginManager::SetPluginEnabled(plugin.Name, switchState);
                if (auto activeProject = Project::GetActive())
                {
                    auto &config = activeProject->GetConfig();
                    if (switchState)
                    {
                        if (!config.EnabledPlugins.Contains(plugin.Name))
                            config.EnabledPlugins.Add(plugin.Name);
                    }
                    else
                    {
                        config.EnabledPlugins.Remove(plugin.Name);
                    }
                    TEString projPath = Project::GetProjectDirectory() / (config.Name + ".teproj");
                    Project::SaveActive(projPath);
                }
            }

            // Generous vertical spacing between cards (12px)
            TimeGUI::Dummy(TEVector2(0.0f, 12.0f));
            TimeGUI::PopID();
        }

        if (visibleCount == 0 && !plugins.empty())
        {
            TimeGUI::Spacing();
            TimeGUI::TextDisabled("  No plugins matched the search filter.");
        }

        TimeGUI::EndChild();
    }

    TimeGUI::End();
}

TE_REGISTER_EDITOR_PANEL(PluginsPanel);

