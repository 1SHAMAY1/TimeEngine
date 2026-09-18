#include "PreRequisites.h"
#include "Settings/ProjectUISettings.hpp"
#include "Layers/EditorLayer.hpp"
#include "TimeGUI.hpp"

TE_REGISTER_PROJECT_SETTINGS(ProjectUISettings, true);

void ProjectUISettings::OnDrawSettingsUI(Ref<EditorLayer> /*editor*/)
{
    TimeGUI::TextColored(TEColor(0.2f, 0.7f, 1.0f, 1.0f), "Active UI Backend");
    TimeGUI::Separator();
    TimeGUI::Spacing();

    const char *backendNames[] = {"Dear ImGui", "ForgeUI"};

    TimeGUI::PushItemWidth(220.0f);
    if (TimeGUI::Combo("Backend", &m_SelectedIndex, backendNames, 2))
    {
        m_SelectedBackend = static_cast<UIBackendType>(m_SelectedIndex);
    }
    TimeGUI::PopItemWidth();

    TimeGUI::Spacing();
    TimeGUI::TextDisabled("Requires application restart to take effect.");

    TimeGUI::Spacing();
    TimeGUI::Separator();
    TimeGUI::Spacing();

    // Show per-backend info
    if (m_SelectedBackend == UIBackendType::DearImGui)
    {
        TimeGUI::TextColored(TEColor(0.6f, 1.0f, 0.6f, 1.0f), "Dear ImGui");
        TimeGUI::TextDisabled("Full docking, multi-viewport, and ImGui ecosystem support.");
    }
    else
    {
        TimeGUI::TextColored(TEColor(1.0f, 0.8f, 0.3f, 1.0f), "ForgeUI");
        TimeGUI::TextDisabled("Data-oriented immediate-mode UI. MSDF fonts, job-system layout.");
        TimeGUI::TextDisabled("Docking and multi-viewport not yet available.");
    }
}
