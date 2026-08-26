#include "Core/Project/ProjectSerializer.hpp"
#include "Core/PreRequisites.h"
#include "Utils/TEFileSystem.hpp"
#include <fstream>

// =============================================================================
// Project Configuration Serialization (.teproj)
// Plaintext Key-Value Descriptor Format:
//   Project: <ProjectName>
//   StartScene: <RelativePathToScene>
//   AssetDirectory: <RelativePathToAssets>
//   ScriptModulePath: <RelativePathToScriptDll>
//   ThumbnailPath: <RelativePathToThumbnailImage>
// =============================================================================

ProjectSerializer::ProjectSerializer(TERef<Project> project) : m_Project(project) {}

bool ProjectSerializer::Serialize(const TEString &filepath)
{
    auto &config = m_Project->GetConfig();

    std::ofstream hout(filepath.c_str());
    if (hout.is_open())
    {
        hout << "Project: " << config.Name.c_str() << std::endl;
        hout << "StartScene: " << config.StartScene.c_str() << std::endl;
        hout << "AssetDirectory: " << config.AssetDirectory.c_str() << std::endl;
        hout << "ScriptModulePath: " << config.ScriptModulePath.c_str() << std::endl;
        hout << "ThumbnailPath: " << config.ThumbnailPath.c_str() << std::endl;

        if (!config.EnabledPlugins.empty())
        {
            hout << "Plugins: ";
            for (size_t i = 0; i < config.EnabledPlugins.size(); ++i)
            {
                hout << config.EnabledPlugins[i].c_str();
                if (i + 1 < config.EnabledPlugins.size())
                    hout << ", ";
            }
            hout << std::endl;
        }

        hout.close();
        return true;
    }
    return false;
}

bool ProjectSerializer::Deserialize(const TEString &filepath)
{
    auto &config = m_Project->GetConfig();
    config.EnabledPlugins.Clear();

    return TEFileSystem::ForEachLine(filepath,
                                     [&config](const TEString &line)
                                     {
                                         if (line.StartsWith("Project: "))
                                             config.Name = line.Mid(9).Trim();
                                         else if (line.StartsWith("StartScene: "))
                                             config.StartScene = line.Mid(12).Trim();
                                         else if (line.StartsWith("AssetDirectory: "))
                                             config.AssetDirectory = line.Mid(16).Trim();
                                         else if (line.StartsWith("ScriptModulePath: "))
                                             config.ScriptModulePath = line.Mid(18).Trim();
                                         else if (line.StartsWith("ThumbnailPath: "))
                                             config.ThumbnailPath = line.Mid(15).Trim();
                                         else if (line.StartsWith("Plugins: "))
                                         {
                                             TEString pluginsList = line.Mid(9).Trim();
                                             auto tokens = pluginsList.Split(',');
                                             for (auto &tok : tokens)
                                             {
                                                 TEString trimmed = tok.Trim();
                                                 if (!trimmed.empty() && !config.EnabledPlugins.Contains(trimmed))
                                                 {
                                                     config.EnabledPlugins.Add(trimmed);
                                                 }
                                             }
                                         }
                                         return true;
                                     });
}
