#include "Core/Project/Project.hpp"
#include "Layers/LogoLayer.hpp"
#ifdef TE_EDITOR
#include "Layers/EditorLayer.hpp"
#include "Layers/ProjectHubLayer.hpp"
#endif
#include "Utils/PlatformUtils.hpp"
#include <Engine.h>
#include <filesystem>
#include <string>

class Sandbox : public TE::Application
{
public:
    Sandbox(const std::string &startProject)
    {
        TE_CORE_INFO("Sandbox Constructor started.");

        TE::LogoLayer *logoLayer = new TE::LogoLayer();
        logoLayer->LogoFinishedDelegate.Add(
            [this, startProject]()
            {
                if (!startProject.empty() && std::filesystem::exists(startProject))
                {
                    TE_CORE_INFO("Attempting to load project: ", startProject);
                // Load Project Directly
#ifdef TE_EDITOR
                    if (TE::Project::Load(startProject))
                    {
                        TE_CORE_INFO("Project loaded successfully. Pushing EditorLayer.");
                        MarkLayerForAddition(new TE::EditorLayer());
                    }
                    else
                    {
                        TE_CORE_ERROR("Failed to load project from args: ", startProject);
                        MarkLayerForAddition(new TE::ProjectHubLayer());
                    }
#else
                    if (TE::Project::Load(startProject))
                    {
                        TE_CORE_INFO("Project loaded successfully in runtime mode.");
                    }
                    else
                    {
                        TE_CORE_ERROR("Failed to load project from args: ", startProject);
                    }
#endif
                }
                else
                {
#ifdef TE_EDITOR
                    TE_CORE_INFO("No valid project argument. Starting Project Hub.");
                    // Start with the Project Hub (Launcher)
                    MarkLayerForAddition(new TE::ProjectHubLayer());
#else
                    TE_CORE_INFO("No project specified for runtime build. Exiting...");
                    Close();
#endif
                }
            });

        PushLayer(logoLayer);
    }
};

TE::Application *TE::CreateApplication(int argc, char **argv)
{
    std::string executablePath = TE::PlatformUtils::GetExecutablePath();

    // Auto-register .teproj extension if not already pointing to this executable
    if (!TE::PlatformUtils::IsFileAssociationRegistered(".teproj", executablePath))
    {
        TE_CORE_INFO("Registering .teproj file association to: {0}", executablePath);
        TE::PlatformUtils::RegisterFileAssociation(".teproj", "TimeEngine.Project", executablePath,
                                                   "TimeEngine Project File");
    }

    std::string startProject = "";
    if (argc > 1)
    {
        std::string arg1 = argv[1];
        if (arg1 == "--register")
        {
            TE_CORE_INFO("Registration complete. Exiting...");
            return nullptr;
        }
        startProject = arg1;
    }

    return new Sandbox(startProject);
}
