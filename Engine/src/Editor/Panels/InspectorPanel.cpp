#include "Editor/Panels/InspectorPanel.hpp"
#include "Core/Asset/AssetManager.hpp"
#include "Core/PreRequisites.h"
#include "Core/Project/Project.hpp"
#include "Core/Scene/ComponentRegistry.hpp"
#include "Core/Scene/EntityManager.hpp"
#include "Core/Scene/Scene.hpp"
#include "Core/Scene/TagComponent.hpp"
#include "Core/Scene/TransformComponent.hpp"
#include "Core/Scripting/TScriptAsset.hpp"
#include "Editor/EditorSaveManager.hpp"
#include "Editor/EditorUtils.hpp"
#include "GameFrameWork/TComponent.hpp"
#include "Layers/EditorLayer.hpp"
#include "UI/Widgets/UISearchBar.hpp"
#include "Utils/TEFileSystem.hpp"
#include "Utils/TimeGUI.hpp"

static void DrawTScriptSlots(TComponent *comp, Ref<Scene> activeScene)
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

        if (TimeGUI::CollapsingHeader(displayName, TimeGUITreeNodeFlags_DefaultOpen))
        {
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
                        bool edited = prop.DrawFunc(comp, prop.DisplayName);
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

            DrawTScriptSlots(comp, activeScene);
        }
    }

    TimeGUI::Spacing();

    // ── Add Component Button ─────────────────────────────────────────────────
    TimeGUI::PushStyleColor(TimeGUICol_Button, TEVector4(0.20f, 0.40f, 0.75f, 0.9f));
    if (TimeGUI::Button("+ Add Component", TEVector2(-1, 28)))
    {
        TimeGUI::OpenPopup("AddComponentPopup");
    }
    TimeGUI::PopStyleColor();

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
