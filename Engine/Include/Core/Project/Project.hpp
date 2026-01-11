#pragma once

#include <string>
#include <filesystem>
#include <memory>
#include "Core/Log.h"

namespace TE {

    struct ProjectConfig
    {
        std::string Name = "Untitled";
        std::filesystem::path StartScene;
        std::filesystem::path AssetDirectory;
        std::filesystem::path ScriptModulePath;
        std::filesystem::path ThumbnailPath;
    };

    class TE_API Project
    {
    public:
        static const std::filesystem::path& GetProjectDirectory()
        {
            return s_ActiveProject->m_ProjectDirectory;
        }

        static std::filesystem::path GetAssetDirectory()
        {
            if (s_ActiveProject->m_Config.AssetDirectory.is_absolute())
                return s_ActiveProject->m_Config.AssetDirectory;
            return s_ActiveProject->m_ProjectDirectory / s_ActiveProject->m_Config.AssetDirectory;
        }

        static ProjectConfig& GetConfig()
        {
            return s_ActiveProject->m_Config;
        }

        static std::shared_ptr<Project> GetActive()
        {
            return s_ActiveProject;
        }

        static std::shared_ptr<Project> New();
        static std::shared_ptr<Project> Load(const std::filesystem::path& path);
        
        static bool SaveActive(const std::filesystem::path& path);

    private:
        ProjectConfig m_Config;
        std::filesystem::path m_ProjectDirectory;

        inline static std::shared_ptr<Project> s_ActiveProject;
    };

}
