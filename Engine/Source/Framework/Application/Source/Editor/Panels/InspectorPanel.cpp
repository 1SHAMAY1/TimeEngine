#include "PreRequisites.h"
#include "Editor/Panels/InspectorPanel.hpp"
#include "AssetManager.hpp"
#include "Project/Project.hpp"
#include "ComponentRegistry.hpp"
#include "EntityManager.hpp"
#include "Scene.hpp"
#include "TagComponent.hpp"
#include "TransformComponent.hpp"
#include "TScriptAsset.hpp"
#include "EditorSaveManager.hpp"
#include "EditorUtils.hpp"
#include "TComponent.hpp"
#include "Layers/EditorLayer.hpp"
#include "Widgets/UISearchBar.hpp"
#include "Utils/TEFileSystem.hpp"
#include "TimeGUI.hpp"
#include "Editor/DragDrop/DragDropManager.hpp"

static void DrawTScriptSlots(TComponent *comp, Ref<Scene> activeScene, Ref<EditorLayer> editor)
{
    if (!comp)
        return;

    TimeGUI::PushID(TEString::FromInt64((int64_t)reinterpret_cast<uintptr_t>(comp)));

    TimeGUI::Spacing();
    TimeGUI::Separator();
    TimeGUI::TextColored(TEColor(0.9f, 0.75f, 0.3f, 1.0f), "TScript Slots");

    auto &scripts = comp->GetScripts();
    int toRemove = -1;
    for (int i = 0; i < scripts.size(); i++)
    {
        auto &slot = scripts[i];
        auto asset = AssetManager::GetAsset<TScriptAsset>(slot.ScriptHandle);
        TEString scriptName =
            asset ? asset->GetName() : ("Script #" + TEString::FromInt64(static_cast<int64_t>(slot.ScriptHandle)));

        TimeGUI::PushID(i);
        if (TimeGUI::Checkbox("##Enabled", &slot.Enabled))
        {
            if (activeScene)
                activeScene->MarkDirty(true);
        }
        TimeGUI::SameLine();
        TimeGUI::TextColored(TEColor(0.85f, 0.65f, 1.0f, 1.0f), "%s", scriptName.c_str());
        TimeGUI::SameLine();
        if (TimeGUI::SmallButton("Remove"))
        {
            toRemove = i;
        }
        TimeGUI::PopID();
    }

    if (toRemove >= 0)
    {
        comp->RemoveScriptAt(toRemove);
        if (activeScene)
            activeScene->MarkDirty(true);
    }

    TEString popupName = "AddScriptPopup_" + TEString::FromInt64((int64_t)reinterpret_cast<uintptr_t>(comp));
    if (TimeGUI::Button("+ Add Script##Btn"))
    {
        TimeGUI::OpenPopup(popupName);
    }

    // Drag Drop target on "+ Add Script" button accepting .tscript assets
    if (TimeGUI::BeginDragDropTarget())
    {
        if (const auto *payload = TimeGUI::AcceptDragDropPayload(TE_DND_ASSET_PATH))
        {
            TEString path((const char *)payload->Data, payload->DataSize);
            if (path.EndsWith(".tscript"))
            {
                AssetHandle handle = AssetManager::LoadAsset(path);
                if (handle != 0)
                {
                    comp->AddScript(handle);
                    if (activeScene)
                        activeScene->MarkDirty(true);
                }
            }
        }
        TimeGUI::EndDragDropTarget();
    }

    if (TimeGUI::BeginPopup(popupName))
    {
        TEString assetDir = Project::GetAssetDirectory();
        if (!assetDir.empty() && TEFileSystem::Exists(assetDir))
        {
            auto files = TEFileSystem::GetFiles(assetDir, "", true);
            bool foundAny = false;
            for (const auto &file : files)
            {
                if (file.GetExtension() == ".tscript")
                {
                    foundAny = true;
                    if (TimeGUI::MenuItem(file.GetFilename().c_str()))
                    {
                        AssetHandle handle = AssetManager::LoadAsset(file);
                        comp->AddScript(handle);
                        if (activeScene)
                            activeScene->MarkDirty(true);
                    }
                }
            }
            if (!foundAny)
            {
                TimeGUI::TextDisabled("No .tscript files found.");
            }
        }
        else
        {
            TimeGUI::TextDisabled("No asset directory loaded.");
        }
        TimeGUI::EndPopup();
    }

    // Event Visibility Badge
    uint32_t mask = comp->GetEventVisibilityMask();
    if (mask != 0)
    {
        TimeGUI::SameLine();
        if (mask & static_cast<uint32_t>(TScriptEventType::CollisionEvent))
        {
            TimeGUI::TextColored(TEColor(0.2f, 0.9f, 0.4f, 1.0f), "[CollisionEvent: VISIBLE]");
        }
        if (mask & static_cast<uint32_t>(TScriptEventType::InputEvent))
        {
            TimeGUI::TextColored(TEColor(0.2f, 0.7f, 1.0f, 1.0f), "[InputEvent: VISIBLE]");
        }
        if (mask & static_cast<uint32_t>(TScriptEventType::AreaEvent))
        {
            TimeGUI::TextColored(TEColor(1.0f, 0.7f, 0.2f, 1.0f), "[AreaEvent: VISIBLE]");
        }
    }

    TimeGUI::PopID();
}

InspectorPanel::InspectorPanel() : IEditorPanel("Inspector")
{
    m_SearchBar = CreateRef<UISearchBar>("Search properties...", "##InspectorSearchBar");
}

void InspectorPanel::OnTimeGUIRender(Ref<EditorLayer> editor)
{
    if (!editor || !m_Visible)
        return;

    TimeGUI::Begin(GetTitle().c_str(), &m_Visible);

    auto activeScene = editor->GetActiveScene();
    if (!activeScene)
    {
        TimeGUI::TextDisabled("No active scene.");
        TimeGUI::End();
        return;
    }

    const auto &selectedEntities = editor->GetSelectedEntities();
    if (selectedEntities.empty())
    {
        TimeGUI::TextDisabled("Select an entity to view properties.");
        TimeGUI::End();
        return;
    }

    Entity primaryEntity = *selectedEntities.begin();
    auto &em = activeScene->GetEntityManager();

    // ── Entity Header ────────────────────────────────────────────────────────
    TEString tagStr = "Entity";
    if (auto *tag = em.GetComponent<TagComponent>(primaryEntity))
    {
        tagStr = tag->Tag;
    }

    TimeGUI::SetNextItemWidth(-1);
    if (TimeGUI::InputText("##EntityTagInput", tagStr))
    {
        if (auto *tag = em.GetComponent<TagComponent>(primaryEntity))
        {
            tag->Tag = tagStr;
            activeScene->MarkDirty(true);
        }
    }

    TimeGUI::Separator();

    // Search bar for properties
    if (m_SearchBar)
    {
        m_SearchBar->Draw();
        TimeGUI::Spacing();
    }

    // ── Render Attached Components ───────────────────────────────────────────
    auto components = em.GetAllComponents(primaryEntity.GetID());
    for (auto *comp : components)
    {
        if (!comp)
            continue;

        TEString compName = comp->GetClassName();
        auto *meta = ComponentRegistry::Get().GetMetadata(compName);
        TEString displayName = (meta && !meta->DisplayName.empty()) ? meta->DisplayName : compName;

        if (m_SearchBar && !m_SearchBar->Matches(displayName))
            continue;

        TEString headerLabel = displayName + "###" + compName + "_" + TEString::FromInt64((uint64_t)comp);
        if (TimeGUI::CollapsingHeader(headerLabel, TimeGUITreeNodeFlags_DefaultOpen))
        {
            TimeGUI::PushID(headerLabel);
            if (meta)
            {
                for (const auto &prop : meta->Properties)
                {
                    if (prop.Condition && !prop.Condition(comp))
                        continue;

                    if (m_SearchBar && !m_SearchBar->Matches(prop.DisplayName) && !m_SearchBar->Matches(prop.Name))
                        continue;

                    if (prop.DrawFunc)
                    {
                        TimeGUI::PushID(prop.Name.c_str());
                        bool edited = prop.DrawFunc(comp, prop.DisplayName);
                        TimeGUI::PopID();
                        if (edited || TimeGUI::IsItemDeactivatedAfterEdit() || TimeGUI::IsItemActive())
                        {
                            activeScene->MarkDirty(true);
                        }
                    }
                }
            }

            comp->OnDrawInspector();
            if (TimeGUI::IsItemDeactivatedAfterEdit() || TimeGUI::IsItemActive())
            {
                activeScene->MarkDirty(true);
            }

            DrawTScriptSlots(comp, activeScene, editor);
            TimeGUI::PopID();
        }
    }

    TimeGUI::Spacing();

    // ── Add Component Button ─────────────────────────────────────────────────
    if (TimeGUI::Button("+ Add Component", TEVector2(-1, 28)))
    {
        TimeGUI::OpenPopup("AddComponentPopup");
    }

    if (TimeGUI::BeginPopup("AddComponentPopup"))
    {
        for (const auto &[className, meta] : ComponentRegistry::Get().GetComponents())
        {
            if (TimeGUI::MenuItem(meta.DisplayName.empty() ? className : meta.DisplayName))
            {
                if (meta.Factory)
                {
                    meta.Factory(&em, primaryEntity.GetID());
                    activeScene->MarkDirty(true);
                }
            }
        }
        TimeGUI::EndPopup();
    }

    TimeGUI::End();
}

TE_REGISTER_EDITOR_PANEL(InspectorPanel);
