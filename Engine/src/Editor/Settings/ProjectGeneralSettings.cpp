#include "Core/PreRequisites.h"
#include "Editor/Settings/ProjectGeneralSettings.hpp"
#include "Core/Project/Project.hpp"
#include "Core/Project/ProjectSerializer.hpp"
#include "Layers/EditorLayer.hpp"
#include "Utils/PlatformUtils.hpp"
#include "Utils/TimeGUI.hpp"

TE_REGISTER_PROJECT_SETTINGS(ProjectGeneralSettings);

void ProjectGeneralSettings::OnDrawSettingsUI(Ref<EditorLayer> editor)
{
    auto activeProject = Project::GetActive();
    if (!activeProject)
    {
        TimeGUI::TextDisabled("No active project loaded.");
        return;
    }

    auto &config = activeProject->GetConfig();

    TimeGUI::TextColored(TEColor(0.2f, 0.7f, 1.0f, 1.0f), "Project Information");
    TimeGUI::Separator();

    TimeGUI::InputText("Project Name", config.Name);

    TimeGUI::Spacing();
    TimeGUI::TextColored(TEColor(0.2f, 0.7f, 1.0f, 1.0f), "Startup & Paths");
    TimeGUI::Separator();

    // Start Scene
    TimeGUI::InputText("Start Scene", config.StartScene);
    TimeGUI::SameLine();
    if (TimeGUI::Button("Browse##StartScene"))
    {
        TEString file = PlatformUtils::OpenFile("TimeEngine Scene (*.tescene)\0*.tescene\0All Files (*.*)\0*.*\0");
        if (!file.empty())
        {
            config.StartScene = file;
        }
    }

    // Asset Directory
    TimeGUI::InputText("Asset Directory", config.AssetDirectory);
    TimeGUI::SameLine();
    if (TimeGUI::Button("Browse##AssetDir"))
    {
        TEString folder = PlatformUtils::OpenFolder(Project::GetProjectDirectory());
        if (!folder.empty())
        {
            config.AssetDirectory = folder;
        }
    }

    // Script Module Path
    TimeGUI::InputText("Script Module", config.ScriptModulePath);
    TimeGUI::SameLine();
    if (TimeGUI::Button("Browse##ScriptModule"))
    {
        TEString file = PlatformUtils::OpenFile("Dynamic Library (*.dll)\0*.dll\0All Files (*.*)\0*.*\0");
        if (!file.empty())
        {
            config.ScriptModulePath = file;
        }
    }

    // Thumbnail Path
    TimeGUI::InputText("Thumbnail Path", config.ThumbnailPath);
    TimeGUI::SameLine();
    if (TimeGUI::Button("Browse##Thumbnail"))
    {
        TEString file = PlatformUtils::OpenFile("Image Files (*.png;*.jpg)\0*.png;*.jpg\0All Files (*.*)\0*.*\0");
        if (!file.empty())
        {
            config.ThumbnailPath = file;
        }
    }

    TimeGUI::Spacing();
    TimeGUI::Separator();

    if (TimeGUI::Button("Save Project Settings", 180.0f, 28.0f))
    {
        TEString projPath = Project::GetProjectDirectory() / (config.Name + ".teproj");
        Project::SaveActive(projPath);
    }
}
