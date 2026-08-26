#pragma once
#include "Core/Log.h"
#include "Utils/TEString.hpp"

struct ProjectConfig
{
    TEString Name = "Untitled";
    TEString StartScene;
    TEString AssetDirectory;
    TEString ScriptModulePath;
    TEString ThumbnailPath;
    TEArray<TEString> EnabledPlugins;
};

class TE_API Project
{
public:
    static const TEString &GetProjectDirectory()
    {
        static TEString s_Empty = "";
        if (!s_ActiveProject)
            return s_Empty;
        return s_ActiveProject->m_ProjectDirectory;
    }

    static TEString GetAssetDirectory()
    {
        if (!s_ActiveProject)
            return "Assets";
        if (s_ActiveProject->m_Config.AssetDirectory.IsAbsolute())
            return s_ActiveProject->m_Config.AssetDirectory;
        return s_ActiveProject->m_ProjectDirectory / s_ActiveProject->m_Config.AssetDirectory;
    }

    ProjectConfig &GetConfig() { return m_Config; }

    static ProjectConfig &GetActiveConfig()
    {
        TE_CORE_ASSERT(s_ActiveProject, "No active project!");
        return s_ActiveProject->m_Config;
    }

    static TERef<Project> GetActive() { return s_ActiveProject; }

    static TERef<Project> New();
    static TERef<Project> Load(const TEString &path);
    static bool SaveActive(const TEString &path);

private:
    ProjectConfig m_Config;
    TEString m_ProjectDirectory;

    inline static TERef<Project> s_ActiveProject;
};
