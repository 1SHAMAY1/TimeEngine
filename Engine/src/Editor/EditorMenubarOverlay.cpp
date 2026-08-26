#include "Core/PreRequisites.h"
#include "Editor/EditorMenubarOverlay.hpp"
#include "Core/Application.h"
#include "Core/Log.h"
#include "Core/Scene/Scene.hpp"
#include "Core/Scene/SceneSerializer.hpp"
#include "Editor/EditorLayoutManager.hpp"
#include "Editor/EditorSaveManager.hpp"
#include "Editor/EditorToolbarOverlay.hpp"
#include "Editor/EditorUtils.hpp"
#include "Editor/Panels/IEditorPanel.hpp"
#include "Layers/EditorLayer.hpp"

static TEArray<TERef<IEditorMenubarOverlay>> s_MenubarOverlays;

void EditorMenubarOverlayRegistry::RegisterOverlay(TERef<IEditorMenubarOverlay> overlay)
{
    if (overlay)
    {
        s_MenubarOverlays.Add(overlay);
    }
}

TEArray<TERef<IEditorMenubarOverlay>> EditorMenubarOverlayRegistry::GetOverlays() { return s_MenubarOverlays; }

void EditorMenubarOverlayRegistry::Clear() { s_MenubarOverlays.Clear(); }

// ── FileMenubarOverlay ───────────────────────────────────────────────────────

void FileMenubarOverlay::RegisterMenubarItems(Ref<EditorLayer> editor)
{
    if (!editor)
        return;

    EditorMenubarItem newItem;
    newItem.id = "File.NewScene";
    newItem.category = "File";
    newItem.label = "New Scene";
    newItem.shortcut = "Ctrl+N";
    newItem.priority = 0;
    newItem.onClick = [editor]()
    {
        auto createScenePrompt = [editor]()
        {
            EditorUtils::OpenFileBrowser("Create New Scene", "Create", "NewScene", ".tescene", true,
                                         [editor](const TEString &chosenPath)
                                         {
                                             auto newScene = CreateRef<Scene>();
                                             newScene->SetName(chosenPath.GetStem());
                                             newScene->SetAssetPath(chosenPath);
                                             newScene->CreateEntity("Main Camera");
                                             SceneSerializer serializer(newScene);
                                             serializer.Serialize(chosenPath);
                                             editor->SetActiveScene(newScene);
                                             editor->ClearSelection();
                                             EditorSaveManager::RegisterSavable(newScene);
                                             TE_CORE_INFO("Created and Loaded New Scene: {0}", chosenPath);
                                         });
        };

        auto activeScene = editor->GetActiveScene();
        if (activeScene && activeScene->IsDirty())
        {
            SaveAllToolbarOverlay::OpenSaveModalWithAction(createScenePrompt);
        }
        else
        {
            createScenePrompt();
        }
    };
    editor->RegisterMenubarItem(newItem);

    EditorMenubarItem saveAllItem;
    saveAllItem.id = "File.SaveAll";
    saveAllItem.category = "File";
    saveAllItem.label = "Save All";
    saveAllItem.shortcut = "Ctrl+S";
    saveAllItem.priority = 1;
    saveAllItem.onClick = [editor]() { SaveAllToolbarOverlay::OpenSaveModal(); };
    editor->RegisterMenubarItem(saveAllItem);

    EditorMenubarItem saveAsItem;
    saveAsItem.id = "File.SaveSceneAs";
    saveAsItem.category = "File";
    saveAsItem.label = "Save Scene As...";
    saveAsItem.shortcut = "Ctrl+Shift+S";
    saveAsItem.priority = 2;
    saveAsItem.onClick = [editor]()
    {
        auto scene = editor->GetActiveScene();
        if (scene)
        {
            EditorUtils::OpenFileBrowser("Save Scene As", "Save", scene->GetName(), ".tescene", true,
                                         [scene](const TEString &chosenPath)
                                         {
                                             scene->SetAssetPath(chosenPath);
                                             scene->SetName(chosenPath.GetStem());
                                             SceneSerializer serializer(scene);
                                             serializer.Serialize(chosenPath);
                                             scene->MarkDirty(false);
                                             TE_CORE_INFO("Scene saved to: {0}", chosenPath);
                                         });
        }
    };
    editor->RegisterMenubarItem(saveAsItem);

    EditorMenubarItem exitItem;
    exitItem.id = "File.Exit";
    exitItem.category = "File";
    exitItem.label = "Exit";
    exitItem.priority = 10;
    exitItem.onClick = []()
    {
        if (EditorSaveManager::HasUnsavedChanges())
        {
            SaveAllToolbarOverlay::OpenSaveModal(true);
        }
        else
        {
            Application::Get().Close();
        }
    };
    editor->RegisterMenubarItem(exitItem);
}

// ── EditMenubarOverlay ───────────────────────────────────────────────────────

void EditMenubarOverlay::RegisterMenubarItems(Ref<EditorLayer> editor)
{
    if (!editor)
        return;

    EditorMenubarItem delItem;
    delItem.id = "Edit.Delete";
    delItem.category = "Edit";
    delItem.label = "Delete Selection";
    delItem.shortcut = "Del";
    delItem.priority = 0;
    delItem.onClick = [editor]() { editor->TriggerDeleteSelectedEntities(); };
    editor->RegisterMenubarItem(delItem);

    EditorMenubarItem editorSettingsItem;
    editorSettingsItem.id = "Edit.EditorSettings";
    editorSettingsItem.category = "Edit";
    editorSettingsItem.label = "Editor Settings...";
    editorSettingsItem.priority = 50;
    editorSettingsItem.isChecked = [editor]() -> bool
    {
        auto panel = editor->GetPanelByID("EditorSettings");
        return panel && panel->IsVisible();
    };
    editorSettingsItem.onClick = [editor]()
    {
        auto panel = editor->GetPanelByID("EditorSettings");
        if (panel)
            panel->SetVisible(!panel->IsVisible());
    };
    editor->RegisterMenubarItem(editorSettingsItem);

    EditorMenubarItem projSettingsItem;
    projSettingsItem.id = "Edit.ProjectSettings";
    projSettingsItem.category = "Edit";
    projSettingsItem.label = "Project Settings...";
    projSettingsItem.priority = 51;
    projSettingsItem.isChecked = [editor]() -> bool
    {
        auto panel = editor->GetPanelByID("ProjectSettings");
        return panel && panel->IsVisible();
    };
    projSettingsItem.onClick = [editor]()
    {
        auto panel = editor->GetPanelByID("ProjectSettings");
        if (panel)
            panel->SetVisible(!panel->IsVisible());
    };
    editor->RegisterMenubarItem(projSettingsItem);

    EditorMenubarItem pluginsItem;
    pluginsItem.id = "Edit.Plugins";
    pluginsItem.category = "Edit";
    pluginsItem.label = "Plugins...";
    pluginsItem.priority = 52;
    pluginsItem.isChecked = [editor]() -> bool
    {
        auto panel = editor->GetPanelByID("Plugins");
        return panel && panel->IsVisible();
    };
    pluginsItem.onClick = [editor]()
    {
        auto panel = editor->GetPanelByID("Plugins");
        if (panel)
            panel->SetVisible(!panel->IsVisible());
    };
    editor->RegisterMenubarItem(pluginsItem);

    // Edit -> Reset Layout to Default
    EditorMenubarItem resetLayoutItem;
    resetLayoutItem.id = "Edit.ResetLayout";
    resetLayoutItem.category = "Edit";
    resetLayoutItem.label = "Reset Layout to Default";
    resetLayoutItem.priority = 60;
    resetLayoutItem.onClick = [editor]() { EditorLayoutManager::Get().ResetToDefaultLayout(editor); };
    editor->RegisterMenubarItem(resetLayoutItem);
}

// Auto-register menubar overlays
TE_REGISTER_MENUBAR_OVERLAY(FileMenubarOverlay);
TE_REGISTER_MENUBAR_OVERLAY(EditMenubarOverlay);
