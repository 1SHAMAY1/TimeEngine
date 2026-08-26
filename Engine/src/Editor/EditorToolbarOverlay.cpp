#include "Editor/EditorToolbarOverlay.hpp"
#include "Core/Application.h"
#include "Core/Log.h"
#include "Core/PreRequisites.h"
#include "Core/Project/Project.hpp"
#include "Core/Scene/SceneSerializer.hpp"
#include "Editor/EditorMode.hpp"
#include "Editor/EditorSaveManager.hpp"
#include "Editor/EditorUtils.hpp"
#include "Layers/EditorLayer.hpp"
#include "Utils/PlatformUtils.hpp"
#include "Utils/TEFileSystem.hpp"
#include "Utils/TimeGUI.hpp"
#include <unordered_map>

static TEArray<TERef<IEditorToolbarOverlay>> s_ToolbarOverlays;

void EditorToolbarOverlayRegistry::RegisterOverlay(TERef<IEditorToolbarOverlay> overlay)
{
    if (overlay)
        s_ToolbarOverlays.Add(overlay);
}

TEArray<TERef<IEditorToolbarOverlay>> EditorToolbarOverlayRegistry::GetOverlays() { return s_ToolbarOverlays; }

void EditorToolbarOverlayRegistry::Clear() { s_ToolbarOverlays.Clear(); }

TE_REGISTER_TOOLBAR_OVERLAY(SaveAllToolbarOverlay);
TE_REGISTER_TOOLBAR_OVERLAY(EditorModeSelectorOverlay);
TE_REGISTER_TOOLBAR_OVERLAY(PlayToolbarOverlay);
TE_REGISTER_TOOLBAR_OVERLAY(PauseToolbarOverlay);
TE_REGISTER_TOOLBAR_OVERLAY(StopToolbarOverlay);
TE_REGISTER_TOOLBAR_OVERLAY(StandaloneToolbarOverlay);
TE_REGISTER_TOOLBAR_OVERLAY(RestartEditorToolbarOverlay);

static bool s_ShowSaveModal = false;
static bool s_PendingOpenSaveModal = false;
static bool s_IsAppExitCloseModal = false;
static bool s_IsSavingAndExiting = false;
static bool s_IsRestartPending = false;
static float s_ExitTimer = 0.0f;
static TEMap<TEString, bool> s_SaveSelectionMap;
static std::function<void()> s_OnSaveProceedCallback = nullptr;

void SaveAllToolbarOverlay::OpenSaveModal(bool isAppExit)
{
    s_OnSaveProceedCallback = nullptr;
    auto dirtyList = EditorSaveManager::GetDirtySavables();
    // If not app exit and only 1 dirty item with empty path (e.g. Untitled Scene), directly prompt Save-As file browser
    if (!isAppExit && dirtyList.Num() == 1)
    {
        auto &onlySavable = dirtyList[0];
        if (onlySavable && onlySavable->GetSavablePath().IsEmpty())
        {
            onlySavable->Save();
            return;
        }
    }

    s_ShowSaveModal = true;
    s_PendingOpenSaveModal = true;
    s_IsAppExitCloseModal = isAppExit;
    s_SaveSelectionMap.clear();
    for (size_t i = 0; i < dirtyList.Num(); ++i)
    {
        auto &savable = dirtyList[i];
        if (savable)
            s_SaveSelectionMap[savable->GetSavableID()] = true;
    }
}

void SaveAllToolbarOverlay::OpenSaveModalWithAction(std::function<void()> onProceed)
{
    s_OnSaveProceedCallback = onProceed;
    s_ShowSaveModal = true;
    s_PendingOpenSaveModal = true;
    s_IsAppExitCloseModal = false;
    s_SaveSelectionMap.clear();
    auto dirtyList = EditorSaveManager::GetDirtySavables();
    for (size_t i = 0; i < dirtyList.Num(); ++i)
    {
        auto &savable = dirtyList[i];
        if (savable)
            s_SaveSelectionMap[savable->GetSavableID()] = true;
    }
}

bool SaveAllToolbarOverlay::OnShortcut(const TEString &shortcutId, Ref<EditorLayer> editor)
{
    if (shortcutId == "Editor_SaveAll")
    {
        OpenSaveModal();
        return true;
    }
    return false;
}

void SaveAllToolbarOverlay::RegisterToolbarItems(Ref<EditorLayer> editor)
{
    EditorToolbarItem item;
    item.id = "SaveAllButton";
    item.label = "Save All";
    item.alignment = EditorToolbarAlignment::Left;
    item.priority = -10;
    item.width = 36.0f;
    item.onCustomRender = [this, editor]() { OnCustomRender("SaveAllButton", editor); };
    EditorToolbarRegistry::RegisterItem(item);
}

void SaveAllToolbarOverlay::OnCustomRender(const TEString &itemId, Ref<EditorLayer> editor)
{
    bool hasDirty = EditorSaveManager::HasUnsavedChanges();
    size_t dirtyCount = EditorSaveManager::GetDirtyCount();

    float btnWidth = 36.0f;
    float btnHeight = 30.0f;
    TEString btnLabel = "##SaveAllButton";

    if (hasDirty)
    {
        TimeGUI::PushStyleColor(TimeGUICol_Button, TEVector4(0.16f, 0.36f, 0.65f, 0.95f));
        TimeGUI::PushStyleColor(TimeGUICol_ButtonHovered, TEVector4(0.22f, 0.46f, 0.80f, 1.0f));
    }
    else
    {
        TimeGUI::PushStyleColor(TimeGUICol_Button, TEVector4(0.13f, 0.16f, 0.20f, 0.90f));
        TimeGUI::PushStyleColor(TimeGUICol_ButtonHovered, TEVector4(0.20f, 0.25f, 0.32f, 1.0f));
    }

    TimeGUI::SetCursorPosY(TimeGUI::GetCursorPosY() + 1.0f);
    TEVector2 btnScreenPos = TimeGUI::GetCursorScreenPos();
    bool clicked = TimeGUI::Button(btnLabel, TEVector2(btnWidth, btnHeight));

    TimeGUI::PopStyleColor(2);

    // Draw centered game engine save / floppy icon inside the icon button
    float iconSize = 22.0f;
    TEVector2 iconMin(btnScreenPos.x + (btnWidth - iconSize) * 0.5f, btnScreenPos.y + (btnHeight - iconSize) * 0.5f);
    TEVector2 iconMax(iconMin.x + iconSize, iconMin.y + iconSize);
    EditorUtils::DrawSaveIcon(iconMin, iconMax, hasDirty);

    if (hasDirty && TimeGUI::IsItemHovered())
    {
        TimeGUI::SetTooltip(("Save All Modified Files (" + TEString::FromInt64(static_cast<int64_t>(dirtyCount)) +
                             " unsaved)\nClick to review and save changes.")
                                .c_str());
    }
    else if (!hasDirty && TimeGUI::IsItemHovered())
    {
        TimeGUI::SetTooltip("Save All (All files saved)");
    }

    if (clicked)
    {
        OpenSaveModal();
    }

    // ── Save Changes Modal Dialog ───────────────────────────────────
    if (s_PendingOpenSaveModal)
    {
        TimeGUI::OpenPopup("Save Content");
        s_PendingOpenSaveModal = false;
    }

    TimeGUI::SetNextWindowSize(TEVector2(640.0f, 460.0f), TimeGUI::TimeGUICond_FirstUseEver);
    if (TimeGUI::BeginPopupModal("Save Content", &s_ShowSaveModal, TimeGUI::TimeGUIWindowFlags_NoResize))
    {
        // Header with Save Icon
        TEVector2 headerIconPos = TimeGUI::GetCursorScreenPos();
        EditorUtils::DrawSaveIcon(TEVector2(headerIconPos.x, headerIconPos.y + 2.0f),
                                  TEVector2(headerIconPos.x + 18.0f, headerIconPos.y + 20.0f), hasDirty);
        TimeGUI::SetCursorPosX(TimeGUI::GetCursorPosX() + 24.0f);
        TimeGUI::TextColored(TEColor(0.95f, 0.95f, 0.95f, 1.0f), TEString("Save Content / Modified Files"));
        TimeGUI::TextDisabled("Select the files/assets you wish to save to disk.");
        TimeGUI::Separator();

        auto dirtyList = EditorSaveManager::GetDirtySavables();

        // Selection Controls Header
        if (TimeGUI::Button("Select All", TEVector2(90.0f, 22.0f)))
        {
            for (size_t i = 0; i < dirtyList.Num(); ++i)
            {
                auto &savable = dirtyList[i];
                if (savable)
                    s_SaveSelectionMap[savable->GetSavableID()] = true;
            }
        }
        TimeGUI::SameLine(0, 8);
        if (TimeGUI::Button("Deselect All", TEVector2(90.0f, 22.0f)))
        {
            for (size_t i = 0; i < dirtyList.Num(); ++i)
            {
                auto &savable = dirtyList[i];
                if (savable)
                    s_SaveSelectionMap[savable->GetSavableID()] = false;
            }
        }
        TimeGUI::SameLine(0, 15);
        TimeGUI::TextDisabled("%d modified item(s) pending save", (int)dirtyList.Num());

        TimeGUI::Spacing();

        // ── Scrollable Box for Modified Files & Git-Style Diff Summary ────────
        if (TimeGUI::BeginChild("##SaveItemsScrollRegion", TEVector2(0, 260.0f), true,
                                TimeGUI::TimeGUIWindowFlags_AlwaysVerticalScrollbar))
        {
            if (dirtyList.IsEmpty())
            {
                TimeGUI::Spacing();
                TimeGUI::TextDisabled("  No modified or unsaved files found.");
            }
            else
            {
                for (size_t i = 0; i < dirtyList.Num(); ++i)
                {
                    auto &savable = dirtyList[i];
                    if (!savable)
                        continue;

                    TEString id = savable->GetSavableID();
                    bool &isSelected = s_SaveSelectionMap[id];

                    TimeGUI::PushID((int)i);

                    // Checkbox to select/deselect
                    TimeGUI::Checkbox("##Select", &isSelected);
                    TimeGUI::SameLine(0, 8);

                    // Status Tag Badge
                    TimeGUI::TextColored(TEColor(0.95f, 0.70f, 0.20f, 1.0f), TEString("[MODIFIED]"));
                    TimeGUI::SameLine(0, 8);

                    // Display Name
                    TimeGUI::TextColored(TEColor(1.0f, 1.0f, 1.0f, 1.0f), savable->GetSavableDisplayName());
                    TimeGUI::SameLine(0, 10);

                    // Type Badge
                    TimeGUI::TextDisabled("[%s]", savable->GetSavableType().c_str());

                    // Path Subtitle & Diff Info
                    TEString pathStr = savable->GetSavablePath();
                    if (pathStr.IsEmpty())
                        pathStr = "(In-Memory Scene / Virtual Asset)";
                    TimeGUI::TextDisabled("    Path: %s", pathStr.c_str());

                    // Diff / Change summary note
                    TimeGUI::TextColored(
                        TEColor(0.45f, 0.75f, 0.95f, 0.90f),
                        TEString("    -> Changes: Asset modified in editor session (pending disk write)"));

                    TimeGUI::Separator();
                    TimeGUI::PopID();
                }
            }
        }
        TimeGUI::EndChild();

        TimeGUI::Spacing();
        TimeGUI::Separator();
        TimeGUI::Spacing();

        // ── Bottom Action Controls (Outside the Scroll Box) ───────────────────
        if (TimeGUI::Button("Cancel", TEVector2(100.0f, 28.0f)))
        {
            s_ShowSaveModal = false;
            s_IsAppExitCloseModal = false;
            s_IsRestartPending = false;
            s_OnSaveProceedCallback = nullptr;
            TimeGUI::CloseCurrentPopup();
        }

        if (s_IsAppExitCloseModal || s_OnSaveProceedCallback != nullptr)
        {
            TimeGUI::SameLine(0, 8);
            TimeGUI::PushStyleColor(TimeGUICol_Button, TEVector4(0.55f, 0.22f, 0.22f, 0.95f));
            TimeGUI::PushStyleColor(TimeGUICol_ButtonHovered, TEVector4(0.70f, 0.28f, 0.28f, 1.0f));
            if (TimeGUI::Button("Don't Save", TEVector2(100.0f, 28.0f)))
            {
                s_ShowSaveModal = false;
                TimeGUI::CloseCurrentPopup();
                if (s_IsAppExitCloseModal)
                {
                    s_IsAppExitCloseModal = false;
                    if (s_IsRestartPending)
                    {
                        s_IsRestartPending = false;
                        RestartEditorToolbarOverlay::RestartEditor();
                    }
                    else
                    {
                        Application::Get().ForceClose();
                    }
                }
                else if (s_OnSaveProceedCallback)
                {
                    auto action = s_OnSaveProceedCallback;
                    s_OnSaveProceedCallback = nullptr;
                    action();
                }
            }
            TimeGUI::PopStyleColor(2);
        }

        TimeGUI::SameLine(0, 12);

        // Save Selected (Proceed)
        TimeGUI::PushStyleColor(TimeGUICol_Button, TEVector4(0.18f, 0.52f, 0.28f, 0.95f));
        TimeGUI::PushStyleColor(TimeGUICol_ButtonHovered, TEVector4(0.22f, 0.62f, 0.35f, 1.0f));
        if (TimeGUI::Button("Save Selected", TEVector2(140.0f, 28.0f)))
        {
            for (size_t i = 0; i < dirtyList.Num(); ++i)
            {
                auto &savable = dirtyList[i];
                if (savable && s_SaveSelectionMap[savable->GetSavableID()])
                {
                    savable->Save();
                }
            }
            s_ShowSaveModal = false;
            TimeGUI::CloseCurrentPopup();
            if (s_IsAppExitCloseModal)
            {
                s_IsSavingAndExiting = true;
                s_ExitTimer = 0.0f;
            }
            else if (s_OnSaveProceedCallback)
            {
                auto action = s_OnSaveProceedCallback;
                s_OnSaveProceedCallback = nullptr;
                action();
            }
        }
        TimeGUI::PopStyleColor(2);

        TimeGUI::SameLine(0, 8);

        // Save All Button
        TimeGUI::PushStyleColor(TimeGUICol_Button, TEVector4(0.20f, 0.40f, 0.75f, 0.95f));
        TimeGUI::PushStyleColor(TimeGUICol_ButtonHovered, TEVector4(0.26f, 0.50f, 0.90f, 1.0f));
        if (TimeGUI::Button("Save All", TEVector2(100.0f, 28.0f)))
        {
            EditorSaveManager::SaveAll();
            s_ShowSaveModal = false;
            TimeGUI::CloseCurrentPopup();
            if (s_IsAppExitCloseModal)
            {
                s_IsSavingAndExiting = true;
                s_ExitTimer = 0.0f;
            }
            else if (s_OnSaveProceedCallback)
            {
                auto action = s_OnSaveProceedCallback;
                s_OnSaveProceedCallback = nullptr;
                action();
            }
        }
        TimeGUI::PopStyleColor(2);

        TimeGUI::EndPopup();
    }

    // ── Saving & Shutting Down Loading Overlay ───────────────────────
    if (s_IsSavingAndExiting)
    {
        s_ExitTimer += TimeGUI::GetIO().DeltaTime;
        TimeGUI::OpenPopup("Shutting Down Modal");
        TimeGUIViewport vp = TimeGUI::GetMainViewport();
        TEVector2 center(vp.Pos.x + vp.Size.x * 0.5f, vp.Pos.y + vp.Size.y * 0.5f);
        TimeGUI::SetNextWindowPos(center, TimeGUI::TimeGUICond_Always, TEVector2(0.5f, 0.5f));
        TimeGUI::SetNextWindowSize(TEVector2(380.0f, 100.0f));
        if (TimeGUI::BeginPopupModal("Shutting Down Modal", nullptr,
                                     TimeGUI::TimeGUIWindowFlags_NoResize | TimeGUI::TimeGUIWindowFlags_NoMove |
                                         TimeGUI::TimeGUIWindowFlags_NoTitleBar))
        {
            TimeGUI::Spacing();
            TimeGUI::TextColored(TEColor(0.25f, 0.85f, 0.45f, 1.0f), s_IsRestartPending
                                                                         ? "  Saving changes & restarting TimeEngine..."
                                                                         : "  Saving changes & closing TimeEngine...");
            TimeGUI::Spacing();
            float progress = (s_ExitTimer < 0.35f) ? (s_ExitTimer / 0.35f) : 1.0f;
            TEVector2 pMin = TimeGUI::GetCursorScreenPos();
            float barW = TimeGUI::GetContentRegionAvail().x;
            float barH = 16.0f;
            TimeGUIDrawList dl = TimeGUI::GetWindowDrawList();
            dl.AddRectFilled(pMin, TEVector2(pMin.x + barW, pMin.y + barH), IM_COL32(30, 36, 48, 255), 4.0f);
            dl.AddRectFilled(pMin, TEVector2(pMin.x + barW * progress, pMin.y + barH), IM_COL32(50, 160, 90, 255),
                             4.0f);
            TimeGUI::EndPopup();
        }

        if (s_ExitTimer >= 0.35f)
        {
            s_IsSavingAndExiting = false;
            s_IsAppExitCloseModal = false;
            if (s_IsRestartPending)
            {
                s_IsRestartPending = false;
                RestartEditorToolbarOverlay::RestartEditor();
            }
            else
            {
                Application::Get().ForceClose();
            }
        }
    }

    // ── In-Engine Visual File Browser Dialog ─────────────────────────
    EditorUtils::DrawFileBrowserModal();
}

void EditorModeSelectorOverlay::RegisterToolbarItems(Ref<EditorLayer> editor)
{
    EditorToolbarItem item;
    item.id = "ModeSelector";
    item.label = "Mode Selector";
    item.alignment = EditorToolbarAlignment::Left;
    item.priority = 0;
    item.width = 200.0f;
    item.onCustomRender = [this, editor]() { OnCustomRender("ModeSelector", editor); };
    EditorToolbarRegistry::RegisterItem(item);
}

void EditorModeSelectorOverlay::OnCustomRender(const TEString &itemId, Ref<EditorLayer> editor)
{
    EditorMode *activeMode = EditorModeRegistry::GetActiveMode();
    TEString modeLabel = activeMode ? activeMode->GetName() : "Selection Mode";

    TimeGUI::SetNextItemWidth(200.0f);
    TimeGUI::SetCursorPosY(TimeGUI::GetCursorPosY() + 4.0f);

    if (TimeGUI::BeginCombo("##ModeSelectorCombo", modeLabel.c_str()))
    {
        for (const auto &mode : EditorModeRegistry::GetModes())
        {
            bool isSelected = (activeMode == mode.get());
            if (TimeGUI::Selectable(mode->GetName(), isSelected))
            {
                EditorModeRegistry::SetActiveMode(mode->GetName());
            }
        }
        TimeGUI::EndCombo();
    }
}

void PlayToolbarOverlay::RegisterToolbarItems(Ref<EditorLayer> editor)
{
    EditorToolbarItem item;
    item.id = "PlayButton";
    item.label = "Play";
    item.tooltip = "Start PIE Mode";
    item.alignment = EditorToolbarAlignment::Center;
    item.priority = 0;
    item.width = 32.0f;
    item.isVisible = [editor]() { return editor && editor->GetSceneState() == EditorLayer::SceneState::Edit; };
    item.onClick = [this, editor]() { OnButtonClicked("PlayButton", editor); };
    EditorToolbarRegistry::RegisterItem(item);
}

void PlayToolbarOverlay::OnButtonClicked(const TEString &itemId, Ref<EditorLayer> editor)
{
    TE_CORE_INFO("[Toolbar] Play button clicked!");
    if (editor)
    {
        editor->OnScenePlay();
    }
}

void PauseToolbarOverlay::RegisterToolbarItems(Ref<EditorLayer> editor)
{
    EditorToolbarItem item;
    item.id = "PauseButton";
    item.label = "Pause";
    item.tooltip = "Pause Runtime Simulation";
    item.alignment = EditorToolbarAlignment::Center;
    item.priority = 1;
    item.width = 32.0f;
    item.isVisible = [editor]() { return editor && editor->GetSceneState() != EditorLayer::SceneState::Edit; };
    item.onClick = [this, editor]() { OnButtonClicked("PauseButton", editor); };
    EditorToolbarRegistry::RegisterItem(item);
}

void PauseToolbarOverlay::OnButtonClicked(const TEString &itemId, Ref<EditorLayer> editor)
{
    TE_CORE_INFO("[Toolbar] Pause button clicked!");
    if (editor)
    {
        if (editor->GetSceneState() == EditorLayer::SceneState::Play)
            editor->OnScenePause();
        else
            editor->OnScenePlay();
    }
}

void StopToolbarOverlay::RegisterToolbarItems(Ref<EditorLayer> editor)
{
    EditorToolbarItem item;
    item.id = "StopButton";
    item.label = "Stop";
    item.tooltip = "Stop Simulation";
    item.alignment = EditorToolbarAlignment::Center;
    item.priority = 2;
    item.width = 32.0f;
    item.isVisible = [editor]() { return editor && editor->GetSceneState() != EditorLayer::SceneState::Edit; };
    item.onClick = [this, editor]() { OnButtonClicked("StopButton", editor); };
    EditorToolbarRegistry::RegisterItem(item);
}

void StopToolbarOverlay::OnButtonClicked(const TEString &itemId, Ref<EditorLayer> editor)
{
    TE_CORE_INFO("[Toolbar] Stop button clicked!");
    if (editor)
    {
        editor->OnSceneStop();
    }
}

static uint32_t s_StandalonePID = 0;

void StandaloneToolbarOverlay::RegisterToolbarItems(Ref<EditorLayer> editor)
{
    EditorToolbarItem item;
    item.id = "StandaloneButton";
    item.label = "Standalone";
    item.tooltip = "Launch Standalone Game Process";
    item.alignment = EditorToolbarAlignment::Right;
    item.priority = 0;
    item.width = 90.0f;
    item.onClick = [this, editor]() { OnButtonClicked("StandaloneButton", editor); };
    EditorToolbarRegistry::RegisterItem(item);
}

void StandaloneToolbarOverlay::OnButtonClicked(const TEString &itemId, Ref<EditorLayer> editor)
{
    if (s_StandalonePID != 0 && PlatformUtils::IsProcessRunning(s_StandalonePID))
    {
        TE_CORE_INFO("Standalone game process (PID {0}) is currently active. Terminating...", s_StandalonePID);
        PlatformUtils::KillProcess(s_StandalonePID);
        s_StandalonePID = 0;
        return;
    }

    TEString exePath = PlatformUtils::GetExecutablePath();
    if (exePath.IsEmpty() || !TEFileSystem::Exists(exePath))
    {
        TE_CORE_ERROR("Failed to resolve executable path for Standalone launch.");
        return;
    }

    TEString projectArg = "";
    TEString sceneArg = "";

    if (Project::GetActive())
    {
        TEString projDir = Project::GetProjectDirectory();
        TEString projName = Project::GetActiveConfig().Name;
        TEString potentialProjFile = projDir / (projName + ".teproj");
        if (TEFileSystem::Exists(potentialProjFile))
        {
            projectArg = "--project \"" + potentialProjFile + "\"";
        }
    }

    // Save active scene snapshot for standalone runner
    if (editor && editor->GetActiveScene())
    {
        TEString tempScenePath = "temp_standalone_scene.tescene";
        if (Project::GetActive())
        {
            tempScenePath = Project::GetAssetDirectory() / "temp_standalone_scene.tescene";
        }

        SceneSerializer serializer(editor->GetActiveScene());
        if (serializer.Serialize(tempScenePath))
        {
            sceneArg = "--scene \"" + tempScenePath + "\"";
        }
    }

    TEString cmdArgs = "--game";
    if (!projectArg.IsEmpty())
        cmdArgs += " " + projectArg;
    if (!sceneArg.IsEmpty())
        cmdArgs += " " + sceneArg;

    TE_CORE_INFO("Launching Standalone Game: {0} {1}", exePath, cmdArgs);
    if (PlatformUtils::LaunchProcess(exePath, cmdArgs, &s_StandalonePID))
    {
        TE_CORE_INFO("Standalone Game started successfully with PID {0}", s_StandalonePID);
    }
    else
    {
        TE_CORE_ERROR("Failed to spawn Standalone Game process.");
    }
}

// ── Restart Editor Toolbar Overlay ────────────────────────────────────────────

void RestartEditorToolbarOverlay::RegisterToolbarItems(Ref<EditorLayer> editor)
{
    EditorToolbarItem item;
    item.id = "RestartEditorButton";
    item.label = "##RestartEditorButton";
    item.tooltip = "Restart Editor";
    item.alignment = EditorToolbarAlignment::Right;
    item.priority = 5;
    item.width = 32.0f;
    item.onCustomRender = [this, editor]() { OnCustomRender("RestartEditorButton", editor); };
    EditorToolbarRegistry::RegisterItem(item);
}

void RestartEditorToolbarOverlay::OnCustomRender(const TEString &itemId, Ref<EditorLayer> editor)
{
    float btnWidth = 32.0f;
    float btnHeight = 30.0f;

    TimeGUI::PushStyleColor(TimeGUICol_Button, TEVector4(0.13f, 0.16f, 0.20f, 0.85f));
    TimeGUI::PushStyleColor(TimeGUICol_ButtonHovered, TEVector4(0.20f, 0.28f, 0.40f, 1.0f));
    TimeGUI::PushStyleColor(TimeGUICol_ButtonActive, TEVector4(0.12f, 0.22f, 0.35f, 1.0f));

    TimeGUI::SetCursorPosY(TimeGUI::GetCursorPosY() + 1.0f);
    TEVector2 btnPos = TimeGUI::GetCursorScreenPos();
    bool clicked = TimeGUI::Button("##RestartEditorButton", TEVector2(btnWidth, btnHeight));

    TimeGUI::PopStyleColor(3);

    TimeGUIDrawList dl = TimeGUI::GetWindowDrawList();
    TEVector2 center = TEVector2(btnPos.x + btnWidth * 0.5f, btnPos.y + btnHeight * 0.5f);
    EditorUtils::DrawRestartIcon(dl, center, 18.0f, 0xFFFFFFFF);

    if (TimeGUI::IsItemHovered())
    {
        TimeGUI::SetTooltip("Restart Editor");
    }

    if (clicked)
    {
        RequestRestart();
    }
}

void RestartEditorToolbarOverlay::RequestRestart()
{
    TE_CORE_INFO("[Toolbar] Restart button clicked!");
    s_IsRestartPending = true;
    if (EditorSaveManager::HasUnsavedChanges())
    {
        TE_CORE_INFO("[Toolbar] Unsaved changes detected, opening save modal before restart.");
        SaveAllToolbarOverlay::OpenSaveModal(true);
    }
    else
    {
        TE_CORE_INFO("[Toolbar] No unsaved changes, restarting immediately.");
        s_IsRestartPending = false;
        RestartEditor();
    }
}

void RestartEditorToolbarOverlay::RestartEditor()
{
    TEString exePath = PlatformUtils::GetExecutablePath();
    TEString args = "";
    if (Project::GetActive())
    {
        args = "\"" + Project::GetProjectDirectory() / (Project::GetActiveConfig().Name + ".teproj") + "\"";
    }
    TE_CORE_INFO("Restarting Editor: {0} {1}", exePath, args);
    PlatformUtils::LaunchProcess(exePath, args);
    Application::Get().ForceClose();
}
