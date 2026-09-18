#include "PreRequisites.h"
#include "Editor/ToolbarOverlays/SimulationToolbarOverlay.hpp"
#include "Log.h"
#include "EditorToolbarRegistry.hpp"
#include "Layers/EditorLayer.hpp"

TE_REGISTER_TOOLBAR_OVERLAY(PlayToolbarOverlay);
TE_REGISTER_TOOLBAR_OVERLAY(PauseToolbarOverlay);
TE_REGISTER_TOOLBAR_OVERLAY(StopToolbarOverlay);

// ── Play ─────────────────────────────────────────────────────────────────────

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
        editor->OnScenePlay();
}

// ── Pause ─────────────────────────────────────────────────────────────────────

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

// ── Stop ──────────────────────────────────────────────────────────────────────

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
        editor->OnSceneStop();
}
