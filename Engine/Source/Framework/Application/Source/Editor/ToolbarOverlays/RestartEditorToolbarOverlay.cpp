#include "PreRequisites.h"
#include "Editor/ToolbarOverlays/RestartEditorToolbarOverlay.hpp"
#include "Application.h"
#include "Log.h"
#include "Project/Project.hpp"
#include "EditorSaveManager.hpp"
#include "EditorToolbarRegistry.hpp"
#include "EditorUtils.hpp"
#include "Editor/ToolbarOverlays/SaveAllToolbarOverlay.hpp"
#include "Layers/EditorLayer.hpp"
#include "Utils/PlatformUtils.hpp"
#include "TimeGUI.hpp"

TE_REGISTER_TOOLBAR_OVERLAY(RestartEditorToolbarOverlay);

static bool s_IsRestartPending = false;

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
    TEVector2 btnPos = TimeGUI::GetCursorScreenPos();
    bool clicked = TimeGUI::Button("##RestartEditorButton", TEVector2(btnWidth, btnHeight));
    TimeGUI::SetItemTooltip("Restart Editor");

    TimeGUI::PopStyleColor(3);

    TimeGUIDrawList dl = TimeGUI::GetWindowDrawList();
    TEVector2 center = TEVector2(btnPos.x + btnWidth * 0.5f, btnPos.y + btnHeight * 0.5f);
    EditorUtils::DrawRestartIcon(dl, center, 18.0f, 0xFFFFFFFF);

    if (clicked)
        RequestRestart();
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
        args = "\"" + Project::GetProjectDirectory() / (Project::GetActiveConfig().Name + ".teproj") + "\"";
    TE_CORE_INFO("Restarting Editor: {0} {1}", exePath, args);
    PlatformUtils::LaunchProcess(exePath, args);
    Application::Get().ForceClose();
}
