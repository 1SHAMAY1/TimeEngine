#include "Editor/ProjectSettingsRegistry.hpp"
#include "Core/PreRequisites.h"

ProjectSettingsRegistry &ProjectSettingsRegistry::Instance()
{
    static ProjectSettingsRegistry instance;
    return instance;
}

void ProjectSettingsRegistry::Register(TERef<TEProjectSettings> settings)
{
    if (settings)
    {
        Instance().m_ProjectSettings.Add(settings);
    }
}

const TEArray<TERef<TEProjectSettings>> &ProjectSettingsRegistry::GetSettings() { return Instance().m_ProjectSettings; }

void ProjectSettingsRegistry::Clear() { Instance().m_ProjectSettings.Clear(); }
