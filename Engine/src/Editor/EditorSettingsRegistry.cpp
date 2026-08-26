#include "Core/PreRequisites.h"
#include "Editor/EditorSettingsRegistry.hpp"

EditorSettingsRegistry &EditorSettingsRegistry::Instance()
{
    static EditorSettingsRegistry instance;
    return instance;
}

void EditorSettingsRegistry::Register(TERef<TEEditorSettings> settings)
{
    if (settings)
    {
        Instance().m_EditorSettings.Add(settings);
    }
}

const TEArray<TERef<TEEditorSettings>> &EditorSettingsRegistry::GetSettings() { return Instance().m_EditorSettings; }

void EditorSettingsRegistry::Clear() { Instance().m_EditorSettings.Clear(); }
