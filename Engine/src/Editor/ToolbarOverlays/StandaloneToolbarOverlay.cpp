#include "Core/PreRequisites.h"
#include "Editor/ToolbarOverlays/StandaloneToolbarOverlay.hpp"
#include "Core/Log.h"
#include "Core/Project/Project.hpp"
#include "Core/Scene/SceneSerializer.hpp"
#include "Editor/EditorToolbarRegistry.hpp"
#include "Editor/EditorUtils.hpp"
#include "Layers/EditorLayer.hpp"
#include "Utils/PlatformUtils.hpp"
#include "Utils/TEFileSystem.hpp"

TE_REGISTER_TOOLBAR_OVERLAY(StandaloneToolbarOverlay);

static uint32_t s_StandalonePID = 0;

void StandaloneToolbarOverlay::RegisterToolbarItems(Ref<EditorLayer> editor)
{
    EditorToolbarItem item;
    item.id = "StandaloneButton";
    item.label = "##StandaloneButton";
    item.tooltip = "Launch Standalone Game Process";
    item.alignment = EditorToolbarAlignment::Right;
    item.priority = 0;
    item.width = 34.0f;
    item.onCustomRender = [this, editor]() { OnCustomRender("StandaloneButton", editor); };
    EditorToolbarRegistry::RegisterItem(item);
}

void StandaloneToolbarOverlay::OnCustomRender(const TEString &itemId, Ref<EditorLayer> editor)
{
    float btnWidth = 34.0f;
    float btnHeight = 28.0f;

    bool isRunning = (s_StandalonePID != 0 && PlatformUtils::IsProcessRunning(s_StandalonePID));
    if (isRunning)
    {
        TimeGUI::PushStyleColor(TimeGUICol_Button, TEVector4(0.55f, 0.20f, 0.20f, 0.85f));
        TimeGUI::PushStyleColor(TimeGUICol_ButtonHovered, TEVector4(0.70f, 0.25f, 0.25f, 1.0f));
        TimeGUI::PushStyleColor(TimeGUICol_ButtonActive, TEVector4(0.45f, 0.15f, 0.15f, 1.0f));
    }
    else
    {
        TimeGUI::PushStyleColor(TimeGUICol_Button, TEVector4(0.13f, 0.16f, 0.20f, 0.85f));
        TimeGUI::PushStyleColor(TimeGUICol_ButtonHovered, TEVector4(0.20f, 0.28f, 0.40f, 1.0f));
        TimeGUI::PushStyleColor(TimeGUICol_ButtonActive, TEVector4(0.12f, 0.22f, 0.35f, 1.0f));
    }

    TEVector2 btnPos = TimeGUI::GetCursorScreenPos();
    bool clicked = TimeGUI::Button("##StandaloneButton", TEVector2(btnWidth, btnHeight));
    TimeGUI::SetItemTooltip(isRunning ? "Stop Standalone Process" : "Launch Standalone Process");

    TimeGUI::PopStyleColor(3);

    TimeGUIDrawList dl = TimeGUI::GetWindowDrawList();
    TEVector2 center = TEVector2(btnPos.x + btnWidth * 0.5f, btnPos.y + btnHeight * 0.5f);
    EditorUtils::DrawStandaloneIcon(dl, center, 18.0f, isRunning ? 0xFF6666FF : 0xFFFFFFFF);

    if (clicked)
    {
        OnButtonClicked(itemId, editor);
    }
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
            projectArg = "--project \"" + potentialProjFile + "\"";
    }

    if (editor && editor->GetActiveScene())
    {
        TEString tempScenePath = "temp_standalone_scene.tescene";
        if (Project::GetActive())
            tempScenePath = Project::GetAssetDirectory() / "temp_standalone_scene.tescene";

        SceneSerializer serializer(editor->GetActiveScene());
        if (serializer.Serialize(tempScenePath))
            sceneArg = "--scene \"" + tempScenePath + "\"";
    }

    TEString cmdArgs = "--game";
    if (!projectArg.IsEmpty())
        cmdArgs += " " + projectArg;
    if (!sceneArg.IsEmpty())
        cmdArgs += " " + sceneArg;

    TE_CORE_INFO("Launching Standalone Game: {0} {1}", exePath, cmdArgs);
    if (PlatformUtils::LaunchProcess(exePath, cmdArgs, &s_StandalonePID))
        TE_CORE_INFO("Standalone Game started successfully with PID {0}", s_StandalonePID);
    else
        TE_CORE_ERROR("Failed to spawn Standalone Game process.");
}
