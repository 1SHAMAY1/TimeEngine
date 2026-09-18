#include "PreRequisites.h"
#include "EditorLayoutManager.hpp"
#include "Log.h"
#include "EditorSaveManager.hpp"
#include "Layers/EditorLayer.hpp"
#include "Utils/TEFileSystem.hpp"
#include "TimeGUI.hpp"

#include "Utils/TEFileSystem.hpp"
EditorLayoutManager &EditorLayoutManager::Get()
{
    static EditorLayoutManager s_Instance;
    return s_Instance;
}

void EditorLayoutManager::Init(const TEString &projectConfigDir)
{
    TEFileSystem::CreateDirectories(projectConfigDir);
    m_LayoutIniPath = projectConfigDir / "Layout.ini";

    // Register with universal SaveManager
    EditorSaveManager::RegisterSavable(TERef<ISavable>(&Get(), [](ISavable *) {}));

    if (TEFileSystem::Exists(m_LayoutIniPath))
    {
        Load();
    }
    else
    {
        m_NeedsDefaultRebuild = true;
    }
}

#include "EditorMode.hpp"

void EditorLayoutManager::BuildDefaultTopology(unsigned int dockspaceID)
{
    m_NeedsDefaultRebuild = false;

    TimeGUI::DockBuilderRemoveNode(dockspaceID);
    TimeGUI::DockBuilderAddNode(dockspaceID, TimeGUI::TimeGUIDockNodeFlags_DockSpace);
    TimeGUI::DockBuilderSetNodeSize(dockspaceID, TimeGUI::GetMainViewport().Size);

    unsigned int dock_main_id = dockspaceID;

    // Split Right: 25% width
    unsigned int dock_id_right =
        TimeGUI::DockBuilderSplitNode(dock_main_id, TimeGUI::TimeGUIDir_Right, 0.25f, nullptr, &dock_main_id);

    // Split Bottom: 28% height from central node
    unsigned int dock_id_bottom =
        TimeGUI::DockBuilderSplitNode(dock_main_id, TimeGUI::TimeGUIDir_Down, 0.28f, nullptr, &dock_main_id);

    // Split Right into Top and Bottom (Hierarchy top, Properties bottom)
    unsigned int dock_id_right_bottom =
        TimeGUI::DockBuilderSplitNode(dock_id_right, TimeGUI::TimeGUIDir_Down, 0.55f, nullptr, &dock_id_right);

    // Dock standard panels
    TimeGUI::DockBuilderDockWindow("Viewport", dock_main_id);

    // Dynamically dock all registered fullscreen editor modes
    for (const auto &mode : EditorModeRegistry::GetModes())
    {
        if (mode && mode->WantsFullscreenWorkspace())
        {
            TimeGUI::DockBuilderDockWindow(mode->GetWorkspaceWindowName(), dock_main_id);
        }
    }
    TimeGUI::DockBuilderDockWindow("Scene Hierarchy", dock_id_right);
    TimeGUI::DockBuilderDockWindow("Properties", dock_id_right_bottom);
    TimeGUI::DockBuilderDockWindow("Editor Settings", dock_id_right_bottom);
    TimeGUI::DockBuilderDockWindow("Project Settings", dock_id_right_bottom);
    TimeGUI::DockBuilderDockWindow("Content Browser", dock_id_bottom);
    TimeGUI::DockBuilderDockWindow("Console & Terminal", dock_id_bottom);

    TimeGUI::DockBuilderFinish(dockspaceID);
    TE_CORE_INFO("EditorLayoutManager: Default dock topology created.");
}

void EditorLayoutManager::ResetToDefaultLayout(Ref<EditorLayer> editor)
{
    m_NeedsDefaultRebuild = true;
    MarkDirty(true);
}

bool EditorLayoutManager::Save()
{
    if (m_LayoutIniPath.empty())
        return false;

    TimeGUI::SaveIniSettingsToDisk(m_LayoutIniPath.c_str());
    MarkDirty(false);
    TE_CORE_INFO("EditorLayoutManager: Workspace layout saved to {0}.", m_LayoutIniPath);
    return true;
}

bool EditorLayoutManager::Load()
{
    if (m_LayoutIniPath.empty() || !TEFileSystem::Exists(m_LayoutIniPath))
        return false;

    // Check if Layout.ini contains cross-platform mismatched absolute paths (e.g. Windows drive letters on non-Windows)
    TEString iniContent = TEFileSystem::ReadAllText(m_LayoutIniPath);
    if (!iniContent.empty())
    {
#if !defined(TE_PLATFORM_WINDOWS)
        // If on Linux/Unix and the ini file contains Windows-style path roots (e.g. AssetEditor_[A-Z]:\),
        // strip or sanitize stale asset editor windows to prevent docking table corruptions.
        if (iniContent.Contains(":\\") || iniContent.Contains("AssetEditor_"))
        {
            TEString sanitized;
            TEArray<TEString> lines = iniContent.Split('\n');
            bool skippingSection = false;
            for (const auto &line : lines)
            {
                TEString trimmed = line.Trim();
                if (trimmed.StartsWith("[Window][AssetEditor_") && trimmed.Contains(":\\"))
                {
                    skippingSection = true;
                    continue;
                }
                if (trimmed.StartsWith("[") && skippingSection)
                {
                    skippingSection = false;
                }
                if (!skippingSection)
                {
                    sanitized += line + "\n";
                }
            }
            if (sanitized != iniContent)
            {
                TEFileSystem::WriteAllText(m_LayoutIniPath, sanitized);
            }
        }
#endif
    }

    TimeGUI::LoadIniSettingsFromDisk(m_LayoutIniPath.c_str());
    MarkDirty(false);
    TE_CORE_INFO("EditorLayoutManager: Workspace layout loaded from {0}.", m_LayoutIniPath);
    return true;
}
