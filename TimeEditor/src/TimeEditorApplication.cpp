#include "Core/Project/Project.hpp"
#include "Layers/LogoLayer.hpp"
#ifdef TE_EDITOR
#include "Layers/EditorLayer.hpp"
#include "Layers/ProjectHubLayer.hpp"
#endif
#include "Renderer/RendererContext.hpp"
#include "Utils/PlatformUtils.hpp"
#include <Engine.h>

#ifdef _WIN32
// Enable high-performance discrete GPU by default (must be in the main executable, not a DLL)
extern "C"
{
    __declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001;
    __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}
#endif

class TimeEditor : public TE::Application
{
public:
    TimeEditor(const std::string &startProject)
    {
        TE_CORE_INFO("TimeEditor Constructor started.");

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

    if (!startProject.empty())
    {
        std::filesystem::path projPath = startProject;
        std::filesystem::path configPath = projPath.parent_path() / "config" / "ProjectSettings.ini";
        if (std::filesystem::exists(configPath))
        {
            std::ifstream hin(configPath);
            if (hin.is_open())
            {
                std::string line;
                while (std::getline(hin, line))
                {
                    if (line.find("TargetAPI: ") == 0)
                    {
                        try
                        {
                            int api = std::stoi(line.substr(11));
                            TE::RendererContext::SetAPI((TE::GraphicsAPI)api);
                        }
                        catch (...)
                        {
                        }
                        break;
                    }
                }
                hin.close();
            }
        }
    }

    return new TimeEditor(startProject);
}
