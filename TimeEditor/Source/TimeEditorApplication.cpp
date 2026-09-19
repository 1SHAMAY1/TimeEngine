#include "Plugin/PluginManager.hpp"
#include "Project/Project.hpp"
#include "Layers/LogoLayer.hpp"
#include "Layers/RuntimeLayer.hpp"
#include "Utils/TEFileSystem.hpp"
#ifdef TE_EDITOR
#include "Layers/EditorLayer.hpp"
#include "Layers/ProjectHubLayer.hpp"
#endif
#include "RendererContext.hpp"
#include "Utils/PlatformUtils.hpp"
#include "Application.hpp"
#include "EntryPoint.h"

#ifdef _WIN32
// Enable high-performance discrete GPU by default (must be in the main executable, not a DLL)
extern "C"
{
    __declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001;
    __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}
#endif

class TimeEditor : public Application
{
public:
    TimeEditor(const TEString &startProject, const TEString &startScene = "", bool isStandalone = false)
    {
        TE_CORE_INFO("TimeEditor Constructor started (Standalone: %s).", isStandalone ? "TRUE" : "FALSE");

        if (isStandalone)
        {
            PluginManager::LoadAllDiscoveredPlugins();
            if (!startProject.IsEmpty() && TEFileSystem::Exists(startProject))
            {
                TE_CORE_INFO("Loading project for standalone execution: %s", startProject.c_str());
                Project::Load(startProject);
            }

            TE_CORE_INFO("Launching Standalone RuntimeLayer (Scene: %s)",
                         startScene.IsEmpty() ? "Default" : startScene.c_str());
            PushLayer(CreateRef<RuntimeLayer>(startScene));
            return;
        }

        auto logoLayer = CreateRef<LogoLayer>();
        logoLayer->LogoFinishedDelegate.Add(
            [this, startProject, startScene]()
            {
                if (!startProject.IsEmpty() && TEFileSystem::Exists(startProject))
                {
                    TE_CORE_INFO("Attempting to load project: %s", startProject.c_str());
#ifdef TE_EDITOR
                    if (Project::Load(startProject))
                    {
                        TE_CORE_INFO("Project loaded successfully. Pushing EditorLayer.");
                        MarkLayerForAddition(CreateRef<EditorLayer>(startScene));
                    }
                    else
                    {
                        TE_CORE_ERROR("Failed to load project from args: %s", startProject.c_str());
                        MarkLayerForAddition(CreateRef<ProjectHubLayer>());
                    }
#else
                    if (Project::Load(startProject))
                    {
                        TE_CORE_INFO("Project loaded successfully in runtime mode.");
                        MarkLayerForAddition(CreateRef<RuntimeLayer>(startScene));
                    }
                    else
                    {
                        TE_CORE_ERROR("Failed to load project from args: %s", startProject.c_str());
                    }
#endif
                }
                else
                {
#ifdef TE_EDITOR
                    TE_CORE_INFO("No valid project argument. Starting Project Hub.");
                    // Start with the Project Hub (Launcher)
                    MarkLayerForAddition(CreateRef<ProjectHubLayer>());
#else
                    TE_CORE_INFO("No project specified for runtime build. Exiting...");
                    Close();
#endif
                }
            });

        PushLayer(logoLayer);
    }
};

Scope<Application> CreateApplication(int argc, char **argv)
{
    if (HeadlessTestRunner::ShouldRunTests(argc, argv))
    {
        HeadlessTestRunner::Run(argc, argv);
        return nullptr;
    }

    TEString executablePath = PlatformUtils::GetExecutablePath();

    // Auto-register .teproj extension if not already pointing to this executable
    if (!PlatformUtils::IsFileAssociationRegistered(".teproj", executablePath))
    {
        TE_CORE_INFO("Registering .teproj file association to: %s", executablePath.c_str());
        PlatformUtils::RegisterFileAssociation(".teproj", "TimeEngine.Project", executablePath,
                                               "TimeEngine Project File");
    }

    TEString startProject = "";
    TEString startScene = "";
    bool isStandalone = false;

    // Check if running from a packaged standalone build directory (contains Config/DefaultEngine.ini or Content/)
    TEString appDir = executablePath.GetParentPath();
    TEString defaultEngineIni = appDir / "Config" / "DefaultEngine.ini";
    if (TEFileSystem::Exists(defaultEngineIni))
    {
        isStandalone = true;
        TE_CORE_INFO("Packaged standalone environment detected via %s", defaultEngineIni.c_str());

        // Parse startup settings from DefaultEngine.ini
        TEFileSystem::ForEachLine(defaultEngineIni,
                                  [&](const TEString &line) -> bool
                                  {
                                      if (line.find("StartScene=") == 0)
                                      {
                                          startScene = line.substr(11);
                                          if (startScene.StartsWith("/") || startScene.StartsWith("\\"))
                                              startScene = startScene.Substr(1);
                                          startScene = appDir / startScene;
                                      }
                                      else if (line.find("ProjectName=") == 0)
                                      {
                                          startProject = line.substr(12);
                                      }
                                      else if (line.find("PreferredAPI=") == 0)
                                      {
                                          TEString apiName = line.substr(13);
                                          if (apiName.Contains("DirectX 11"))
                                              RendererContext::SetAPI(GraphicsAPI::DirectX11);
                                          else if (apiName.Contains("Vulkan"))
                                              RendererContext::SetAPI(GraphicsAPI::Vulkan);
                                          else if (apiName.Contains("OpenGL"))
                                              RendererContext::SetAPI(GraphicsAPI::OpenGL);
                                          else if (apiName.Contains("Metal"))
                                              RendererContext::SetAPI(GraphicsAPI::Metal);
                                          else if (apiName.Contains("OpenGLES") || apiName.Contains("WebGL"))
                                              RendererContext::SetAPI(GraphicsAPI::OpenGLES);
                                      }
                                      return true;
                                  });
    }

    for (int i = 1; i < argc; ++i)
    {
        TEString arg = argv[i];
        if (arg == "--register")
        {
            TE_CORE_INFO("Registration complete. Exiting...");
            return nullptr;
        }
        else if (arg == "--game" || arg == "-game" || arg == "--standalone")
        {
            isStandalone = true;
        }
        else if (arg == "--project" && (i + 1) < argc)
        {
            startProject = argv[++i];
        }
        else if (arg == "--scene" && (i + 1) < argc)
        {
            startScene = argv[++i];
        }
        else if (!arg.StartsWith("-"))
        {
            if (arg.EndsWith(".teproj"))
            {
                startProject = arg;
            }
            else if (arg.EndsWith(".tescene"))
            {
                // Find parent project directory
                TEString currentDir = arg.GetParentPath();
                TEString foundProj = "";
                while (!currentDir.empty() && currentDir != currentDir.GetParentPath())
                {
                    auto projFiles = TEFileSystem::GetFiles(currentDir, ".teproj", false);
                    if (!projFiles.IsEmpty())
                    {
                        foundProj = projFiles[0];
                        break;
                    }
                    currentDir = currentDir.GetParentPath();
                }
                if (!foundProj.empty())
                    startProject = foundProj;
                startScene = arg;
            }
            else if (startProject.IsEmpty())
            {
                startProject = arg;
            }
        }
    }

    if (!startProject.IsEmpty())
    {
        TEString projPath = startProject;
        TEString configPath = projPath.GetParentPath() / "config" / "ProjectSettings.ini";
        if (TEFileSystem::Exists(configPath))
        {
            bool found = false;
            TEFileSystem::ForEachLine(configPath,
                                      [&](const TEString &line) -> bool
                                      {
                                          if (found)
                                              return false;
                                          if (line.find("TargetAPI: ") == 0)
                                          {
                                              try
                                              {
                                                  int api = std::stoi(line.substr(11));
                                                  RendererContext::SetAPI((GraphicsAPI)api);
                                              }
                                              catch (...)
                                              {
                                              }
                                              found = true;
                                              return false;
                                          }
                                          return true;
                                      });
        }
    }

    return CreateScope<TimeEditor>(startProject, startScene, isStandalone);
}
