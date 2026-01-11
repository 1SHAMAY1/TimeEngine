#include "Core/Project/ProjectSerializer.hpp"
#include <fstream>


// Simple YAML-based serialization for now. 
// We will need to ensure yaml-cpp is available or use a simple custom parser if not.
// Given the user context didn't explicitly show yaml-cpp in vendor, 
// I'll implementation a very basic custom parser to avoid dependency issues for now, 
// or check if I should use JSON since there are json files in the directory.
// The directory listing showed "Client_TimeEngineLog.json", suggesting JSON support might be present or preferred.
// Let's use nlohmann/json if available, or just simple text for this first pass to be safe and robust.

// Actually, let's just use a simple robust text format for .teproj for now to guarantee no compilation errors 
// without checking vendor deep dive. 
// Format:
// Project: Name
// StartScene: Path/To/Scene
// AssetDirectory: Path/To/Assets
// ScriptModulePath: Path/To/dll

namespace TE {

    ProjectSerializer::ProjectSerializer(std::shared_ptr<Project> project)
        : m_Project(project)
    {
    }

    bool ProjectSerializer::Serialize(const std::filesystem::path& filepath)
    {
        const auto& config = m_Project->GetConfig();

        std::ofstream hout(filepath);
        if (hout.is_open())
        {
            hout << "Project: " << config.Name << std::endl;
            hout << "StartScene: " << config.StartScene.string() << std::endl;
            hout << "AssetDirectory: " << config.AssetDirectory.string() << std::endl;
            hout << "ScriptModulePath: " << config.ScriptModulePath.string() << std::endl;
            hout << "ThumbnailPath: " << config.ThumbnailPath.string() << std::endl;
            hout.close();
            return true;
        }
        return false;
    }

    bool ProjectSerializer::Deserialize(const std::filesystem::path& filepath)
    {
        auto& config = m_Project->GetConfig();

        std::ifstream hin(filepath);
        if (hin.is_open())
        {
            std::string line;
            while (std::getline(hin, line))
            {
                if (line.find("Project: ") == 0)
                    config.Name = line.substr(9);
                else if (line.find("StartScene: ") == 0)
                    config.StartScene = line.substr(12);
                else if (line.find("AssetDirectory: ") == 0)
                    config.AssetDirectory = line.substr(16);
                else if (line.find("ScriptModulePath: ") == 0)
                    config.ScriptModulePath = line.substr(18);
                else if (line.find("ThumbnailPath: ") == 0)
                    config.ThumbnailPath = line.substr(15);
            }
            hin.close();
            return true;
        }

        return false;
    }

}
