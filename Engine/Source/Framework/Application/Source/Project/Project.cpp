#include "PreRequisites.h"
#include "Project/Project.hpp"
#include "Log.h"
#include "Project/ProjectSerializer.hpp"

#include "RendererContext.hpp"
#include "Utils/TEFileSystem.hpp"

TERef<Project> Project::New()
{
    s_ActiveProject = CreateRef<Project>();
    return s_ActiveProject;
}

TERef<Project> Project::Load(const TEString &path)
{
    TERef<Project> project = CreateRef<Project>();

    ProjectSerializer serializer(project);
    if (serializer.Deserialize(path))
    {
        project->m_ProjectDirectory = path.GetParentPath();
        s_ActiveProject = project;
        Log::SetProjectLogDirectory(project->m_ProjectDirectory / "Logs");
        LoadProjectSettings();
        return s_ActiveProject;
    }

    return nullptr;
}

bool Project::SaveActive(const TEString &path)
{
    ProjectSerializer serializer(s_ActiveProject);
    if (serializer.Serialize(path))
    {
        s_ActiveProject->m_ProjectDirectory = path.GetParentPath();
        SaveProjectSettings();
        return true;
    }
    return false;
}

void Project::LoadProjectSettings()
{
    if (!s_ActiveProject)
        return;

    TEString settingsPath = GetProjectDirectory() / "Config/ProjectSettings.ini";
    if (!TEFileSystem::Exists(settingsPath))
        return;

    TEFileSystem::ForEachLine(settingsPath,
                              [](const TEString &line)
                              {
                                  if (line.StartsWith("GraphicsAPI="))
                                  {
                                      TEString apiStr = line.Mid(12).Trim();
                                      if (apiStr == "DirectX11")
                                          RendererContext::SetAPI(GraphicsAPI::DirectX11);
                                      else if (apiStr == "OpenGL")
                                          RendererContext::SetAPI(GraphicsAPI::OpenGL);
                                      else if (apiStr == "Vulkan")
                                          RendererContext::SetAPI(GraphicsAPI::Vulkan);
                                      else if (apiStr == "OpenGLES")
                                          RendererContext::SetAPI(GraphicsAPI::OpenGLES);
                                      else if (apiStr == "Metal")
                                          RendererContext::SetAPI(GraphicsAPI::Metal);
                                  }
                                  return true;
                              });
}

void Project::SaveProjectSettings()
{
    if (!s_ActiveProject)
        return;

    TEString configDir = GetProjectDirectory() / "Config";
    if (!TEFileSystem::Exists(configDir))
        TEFileSystem::CreateDirectories(configDir);

    TEString settingsPath = configDir / "ProjectSettings.ini";
    TEString content = "; =========================================\n"
                       "; TimeEngine Project Configuration File (.ini)\n"
                       "; =========================================\n\n"
                       "[Rendering]\n"
                       "GraphicsAPI=";

    switch (RendererContext::GetAPI())
    {
    case GraphicsAPI::DirectX11:
        content += "DirectX11\n";
        break;
    case GraphicsAPI::OpenGL:
        content += "OpenGL\n";
        break;
    case GraphicsAPI::Vulkan:
        content += "Vulkan\n";
        break;
    case GraphicsAPI::OpenGLES:
        content += "OpenGLES\n";
        break;
    case GraphicsAPI::Metal:
        content += "Metal\n";
        break;
    default:
        content += "DirectX11\n";
        break;
    }

    TEFileSystem::WriteAllText(settingsPath, content);
}
