#include "Core/PreRequisites.h"
#include "Editor/EditorMode.hpp"
#include <algorithm>


EditorModeRegistry &EditorModeRegistry::Instance()
{
    static EditorModeRegistry instance;
    return instance;
}

void EditorModeRegistry::RegisterModeInternal(TEScope<EditorMode> mode)
{
    if (mode)
        Instance().m_Modes.push_back(std::move(mode));
}

void EditorModeRegistry::UnregisterMode(const TEString &name)
{
    auto &instance = Instance();
    if (instance.m_ActiveMode && instance.m_ActiveMode->GetName() == name)
    {
        instance.m_ActiveMode->OnExit();
        instance.m_ActiveMode = nullptr;
    }

    auto it = std::remove_if(instance.m_Modes.begin(), instance.m_Modes.end(),
                             [&name](const TEScope<EditorMode> &mode) {
                                 return mode && mode->GetName() == name;
                             });
    instance.m_Modes.erase(it, instance.m_Modes.end());
}

#include "Utils/TimeGUI.hpp"

void EditorModeRegistry::SetActiveMode(const TEString &name)
{
    auto &instance = Instance();
    if (instance.m_ActiveMode && instance.m_ActiveMode->GetName() == name)
        return;

    for (auto &mode : instance.m_Modes)
    {
        if (mode && mode->GetName() == name)
        {
            if (instance.m_ActiveMode)
                instance.m_ActiveMode->OnExit();
            instance.m_ActiveMode = mode.get();
            if (instance.m_ActiveMode && instance.m_ActiveMode->WantsFullscreenWorkspace())
            {
                TimeGUI::DockBuilderDockWindow(instance.m_ActiveMode->GetWorkspaceWindowName(), TimeGUI::GetID("MyDockSpace"));
            }
            instance.m_ActiveMode->OnEnter();
            return;
        }
    }
}

EditorMode *EditorModeRegistry::GetActiveMode()
{
    return Instance().m_ActiveMode;
}

const TEArray<TEScope<EditorMode>> &EditorModeRegistry::GetModes()
{
    return Instance().m_Modes;
}

void EditorModeRegistry::Clear()
{
    auto &instance = Instance();
    if (instance.m_ActiveMode)
    {
        instance.m_ActiveMode->OnExit();
        instance.m_ActiveMode = nullptr;
    }
    instance.m_Modes.clear();
}

