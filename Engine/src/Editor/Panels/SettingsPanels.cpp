#include "Core/PreRequisites.h"
#include "Editor/Panels/SettingsPanels.hpp"
#include "Editor/EditorSettingsRegistry.hpp"
#include "Editor/ProjectSettingsRegistry.hpp"
#include "Layers/EditorLayer.hpp"
#include "Utils/TimeGUI.hpp"

// ── EditorSettingsPanel ────────────────────────────────────────────────────

void EditorSettingsPanel::OnTimeGUIRender(Ref<EditorLayer> editor)
{
    if (!editor || !m_Visible)
        return;

    TimeGUI::SetNextWindowSize(TEVector2(750, 500), TimeGUICond_FirstUseEver);

    if (TimeGUI::Begin(GetTitle().c_str(), &m_Visible))
    {
        const auto &settingsList = EditorSettingsRegistry::GetSettings();
        if (settingsList.empty())
        {
            TimeGUI::TextDisabled("No editor settings registered.");
            TimeGUI::End();
            return;
        }

        if (m_SelectedIdx < 0 || m_SelectedIdx >= (int)settingsList.size())
        {
            m_SelectedIdx = 0;
        }

        // Left sidebar: Categories & Section Items
        TimeGUI::BeginChild("SettingsSidebar", TEVector2(200.0f, 0.0f), true);
        TimeGUI::TextColored(TEColor(0.8f, 0.8f, 0.8f, 1.0f), "Categories");
        TimeGUI::Separator();

        for (int i = 0; i < (int)settingsList.size(); ++i)
        {
            auto &s = settingsList[i];
            if (!s)
                continue;

            TEString itemLabel = s->GetDisplayName();
            bool isSelected = (m_SelectedIdx == i);
            if (TimeGUI::Selectable(itemLabel, isSelected))
            {
                m_SelectedIdx = i;
            }
        }
        TimeGUI::EndChild();

        TimeGUI::SameLine();

        // Right pane: Active settings inspector
        TimeGUI::BeginChild("SettingsContent", TEVector2(0.0f, 0.0f), true);
        if (m_SelectedIdx >= 0 && m_SelectedIdx < (int)settingsList.size() && settingsList[m_SelectedIdx])
        {
            auto &activeSetting = settingsList[m_SelectedIdx];
            TimeGUI::TextColored(TEColor(0.2f, 0.7f, 1.0f, 1.0f), "%s > %s", activeSetting->GetCategory().c_str(),
                                 activeSetting->GetDisplayName().c_str());
            TimeGUI::Separator();
            TimeGUI::Spacing();

            activeSetting->OnDrawSettingsUI(editor);
        }
        TimeGUI::EndChild();
    }
    TimeGUI::End();
}

// ── ProjectSettingsPanel ───────────────────────────────────────────────────

void ProjectSettingsPanel::OnTimeGUIRender(Ref<EditorLayer> editor)
{
    if (!editor || !m_Visible)
        return;

    TimeGUI::SetNextWindowSize(TEVector2(750, 500), TimeGUICond_FirstUseEver);

    if (TimeGUI::Begin(GetTitle().c_str(), &m_Visible))
    {
        const auto &settingsList = ProjectSettingsRegistry::GetSettings();
        if (settingsList.empty())
        {
            TimeGUI::TextDisabled("No project settings registered.");
            TimeGUI::End();
            return;
        }

        if (m_SelectedIdx < 0 || m_SelectedIdx >= (int)settingsList.size())
        {
            m_SelectedIdx = 0;
        }

        // Left sidebar: Categories & Section Items
        TimeGUI::BeginChild("ProjectSettingsSidebar", TEVector2(200.0f, 0.0f), true);
        TimeGUI::TextColored(TEColor(0.8f, 0.8f, 0.8f, 1.0f), "Categories");
        TimeGUI::Separator();

        for (int i = 0; i < (int)settingsList.size(); ++i)
        {
            auto &s = settingsList[i];
            if (!s)
                continue;

            TEString itemLabel = s->GetDisplayName();
            bool isSelected = (m_SelectedIdx == i);
            if (TimeGUI::Selectable(itemLabel, isSelected))
            {
                m_SelectedIdx = i;
            }
        }
        TimeGUI::EndChild();

        TimeGUI::SameLine();

        // Right pane: Active settings inspector
        TimeGUI::BeginChild("ProjectSettingsContent", TEVector2(0.0f, 0.0f), true);
        if (m_SelectedIdx >= 0 && m_SelectedIdx < (int)settingsList.size() && settingsList[m_SelectedIdx])
        {
            auto &activeSetting = settingsList[m_SelectedIdx];
            TimeGUI::TextColored(TEColor(0.2f, 0.7f, 1.0f, 1.0f), "%s > %s", activeSetting->GetCategory().c_str(),
                                 activeSetting->GetDisplayName().c_str());
            TimeGUI::Separator();
            TimeGUI::Spacing();

            activeSetting->OnDrawSettingsUI(editor);
        }
        TimeGUI::EndChild();
    }
    TimeGUI::End();
}

TE_REGISTER_EDITOR_PANEL(EditorSettingsPanel);
TE_REGISTER_EDITOR_PANEL(ProjectSettingsPanel);
