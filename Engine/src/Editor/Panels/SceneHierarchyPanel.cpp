#include "Core/PreRequisites.h"
#include "Editor/Panels/SceneHierarchyPanel.hpp"
#include "Core/Log.h"
#include "Core/Scene/ComponentRegistry.hpp"
#include "Core/Scene/EntityManager.hpp"
#include "Core/Scene/Scene.hpp"
#include "Core/Scene/TagComponent.hpp"
#include "Core/Scene/TransformComponent.hpp"
#include "Editor/EditorSaveManager.hpp"
#include "Editor/EditorUtils.hpp"
#include "Editor/ViewportContextMenu.hpp"
#include "GameFrameWork/TComponent.hpp"
#include "Layers/EditorLayer.hpp"
#include "UI/Widgets/UISearchBar.hpp"
#include "Utils/TimeGUI.hpp"

static Entity s_ContextEntity = Entity();
static bool s_ShouldOpenHierarchyContextMenu = false;

SceneHierarchyPanel::SceneHierarchyPanel() : IEditorPanel("Scene Hierarchy")
{
    m_SearchBar = CreateRef<UISearchBar>("Search entities...", "##HierarchySearchBar");
    m_CreateModalSearchBar =
        CreateRef<UISearchBar>("Search Entity or Component type...", "##CreateEntityModalSearchBar");
}

void SceneHierarchyPanel::OnTimeGUIRender(Ref<EditorLayer> editor)
{
    if (!editor || !m_Visible)
        return;

    TimeGUI::Begin(GetTitle().c_str(), &m_Visible);

    m_Focused = TimeGUI::IsWindowFocused(TimeGUIFocusedFlags_RootAndChildWindows);
    m_Hovered = TimeGUI::IsWindowHovered(TimeGUIHoveredFlags_RootAndChildWindows);

    auto activeScene = editor->GetActiveScene();
    if (!activeScene)
    {
        TimeGUI::TextDisabled("No active scene.");
        TimeGUI::End();
        return;
    }

    // Top Search Bar
    if (m_SearchBar)
    {
        m_SearchBar->Draw();
        TimeGUI::Spacing();
    }

    auto &em = activeScene->GetEntityManager();
    const auto &aliveEntities = em.GetAliveEntities();

    // ── Professional + Add Entity Button (Opens Modal Dialog) ─────────────────
    TimeGUI::PushStyleColor(TimeGUICol_Button, TEVector4(0.18f, 0.60f, 0.28f, 0.90f));
    TimeGUI::PushStyleColor(TimeGUICol_ButtonHovered, TEVector4(0.24f, 0.72f, 0.35f, 1.0f));
    TimeGUI::PushStyleColor(TimeGUICol_ButtonActive, TEVector4(0.14f, 0.48f, 0.22f, 1.0f));
    if (TimeGUI::Button("+ Add Entity", TEVector2(-1, 28)))
    {
        m_ShowCreateEntityModal = true;
        m_SelectedPresetName = "Empty Entity";
        TimeGUI::OpenPopup("Create New Entity");
    }
    TimeGUI::PopStyleColor(3);

    // ── Categorized Modal Dialog with Search Bar ──────────────────────────────
    TEVector2 displaySize = TimeGUI::GetIO().DisplaySize;
    TEVector2 centerPos = TEVector2(displaySize.x * 0.5f, displaySize.y * 0.5f);
    TimeGUI::SetNextWindowPos(centerPos, TimeGUICond_Appearing, TEVector2(0.5f, 0.5f));
    TimeGUI::SetNextWindowSize(TEVector2(880.0f, 620.0f), TimeGUICond_Appearing);
    if (TimeGUI::BeginPopupModal("Create New Entity", &m_ShowCreateEntityModal))
    {
        // 1. Search Bar at Top
        if (m_CreateModalSearchBar)
        {
            m_CreateModalSearchBar->Draw();
            TimeGUI::Spacing();
        }

        // 2. Split Area: Category Tree (Left 40%) + Details/Preview (Right 60%)
        TimeGUI::Columns(2, "CreateEntityColumns", true);
        TimeGUI::SetColumnWidth(0, 350.0f);

        TimeGUI::BeginChild("##PresetCategoryTree", TEVector2(0, 470.0f), true);

        // Standard "Empty Entity"
        if (!m_CreateModalSearchBar || m_CreateModalSearchBar->Matches("Empty Entity"))
        {
            bool isSelected = (m_SelectedPresetName == "Empty Entity");
            if (TimeGUI::Selectable("  Empty Entity", isSelected))
            {
                m_SelectedPresetName = "Empty Entity";
            }
        }

        // Group presets by category
        TEMap<TEString, TEArray<EntityPreset>> categorizedPresets;
        for (const auto &preset : ComponentRegistry::Get().GetEntityPresets())
        {
            if (m_CreateModalSearchBar && !m_CreateModalSearchBar->Matches(preset.Name) &&
                !m_CreateModalSearchBar->Matches(preset.Category))
                continue;

            TEString cat = preset.Category.empty() ? "General" : preset.Category;
            categorizedPresets[cat].push_back(preset);
        }

        for (const auto &[category, presets] : categorizedPresets)
        {
            if (TimeGUI::TreeNodeEx(category.c_str(), TimeGUITreeNodeFlags_DefaultOpen))
            {
                for (const auto &p : presets)
                {
                    bool isSelected = (m_SelectedPresetName == p.Name);
                    if (TimeGUI::Selectable(("  " + p.Name).c_str(), isSelected))
                    {
                        m_SelectedPresetName = p.Name;
                    }
                }
                TimeGUI::TreePop();
            }
        }

        TimeGUI::EndChild();

        // Right Column: Details & Description
        TimeGUI::NextColumn();
        TimeGUI::BeginChild("##PresetDetails", TEVector2(0, 470.0f), true);
        TimeGUI::TextColored(TEVector4(0.35f, 0.70f, 1.0f, 1.0f), "%s", m_SelectedPresetName.c_str());
        TimeGUI::Separator();

        if (m_SelectedPresetName == "Empty Entity")
        {
            TimeGUI::TextWrapped("A blank entity with a Transform component located at the origin.");
        }
        else
        {
            TimeGUI::TextWrapped("Preset configured with components and defaults for %s.",
                                 m_SelectedPresetName.c_str());
        }

        TimeGUI::EndChild();
        TimeGUI::Columns(1);

        TimeGUI::Separator();
        TimeGUI::Spacing();

        // 3. Footer Action Buttons (Create / Cancel)
        float btnWidth = 110.0f;
        TimeGUI::SetCursorPosX(TimeGUI::GetWindowWidth() - (btnWidth * 2.0f + 24.0f));

        TimeGUI::PushStyleColor(TimeGUICol_Button, TEVector4(0.20f, 0.45f, 0.85f, 0.95f));
        if (TimeGUI::Button("Create", TEVector2(btnWidth, 28.0f)))
        {
            if (m_SelectedPresetName == "Empty Entity")
            {
                Entity e = activeScene->CreateEntity("Empty Entity");
                editor->ClearSelection();
                editor->SelectEntity(e);
                activeScene->MarkDirty(true);
            }
            else
            {
                for (const auto &preset : ComponentRegistry::Get().GetEntityPresets())
                {
                    if (preset.Name == m_SelectedPresetName)
                    {
                        Entity e = activeScene->CreateEntity(preset.Name);
                        if (preset.Create)
                            preset.Create(e.GetID(), &em);
                        editor->ClearSelection();
                        editor->SelectEntity(e);
                        activeScene->MarkDirty(true);
                        break;
                    }
                }
            }
            m_ShowCreateEntityModal = false;
            TimeGUI::CloseCurrentPopup();
        }
        TimeGUI::PopStyleColor();

        TimeGUI::SameLine(0, 8.0f);
        if (TimeGUI::Button("Cancel", TEVector2(btnWidth, 26.0f)))
        {
            m_ShowCreateEntityModal = false;
            TimeGUI::CloseCurrentPopup();
        }

        TimeGUI::EndPopup();
    }

    TimeGUI::Separator();

    // ── Entity Tree Nodes (Recursive hierarchy with action buttons) ───────────
    TimeGUI::PushStyleVar(TimeGUIStyleVar_FramePadding, TEVector2(6.0f, 6.0f));
    TimeGUI::PushStyleVar(TimeGUIStyleVar_ItemSpacing, TEVector2(4.0f, 4.0f));

    Entity entityToDelete;
    for (EntityID entityID : aliveEntities)
    {
        Entity entity(entityID, &em);
        auto *tc = em.GetComponent<TransformComponent>(entity);
        // Only draw root entities at the top level
        if (!tc || tc->Parent == 0 || !em.IsValid(tc->Parent))
        {
            DrawEntityNode(entityID, em, editor, entityToDelete);
        }
    }

    TimeGUI::PopStyleVar(2);

    if (entityToDelete.IsValid())
    {
        activeScene->DestroyEntity(entityToDelete);
        editor->ClearSelection();
        activeScene->MarkDirty(true);
    }

    // Render active Scene Hierarchy Context Menu
    if (s_ShouldOpenHierarchyContextMenu)
    {
        TimeGUI::OpenPopup("SceneHierarchyContextMenu");
        s_ShouldOpenHierarchyContextMenu = false;
    }

    if (TimeGUI::BeginPopup("SceneHierarchyContextMenu"))
    {
        TE_CORE_INFO("[Hierarchy] Rendering SceneHierarchyContextMenu for entity ID: {0}",
                     (uint64_t)s_ContextEntity.GetID());
        bool hasEntity = s_ContextEntity.IsValid() && em.IsValid(s_ContextEntity.GetID());
        if (hasEntity)
        {
            TEString entityName = "Entity (" + TEString::FromInt64(static_cast<int64_t>(s_ContextEntity.GetID())) + ")";
            if (auto *tag = em.GetComponent<TagComponent>(s_ContextEntity))
                entityName = tag->Tag;

            TimeGUI::TextDisabled(entityName);
            TimeGUI::Separator();

            if (TimeGUI::BeginMenu("Add Component"))
            {
                for (const auto &[className, meta] : ComponentRegistry::Get().GetComponents())
                {
                    if (meta.IsInternal)
                        continue;
                    if (TimeGUI::MenuItem(meta.DisplayName.empty() ? className : meta.DisplayName))
                    {
                        if (meta.Factory)
                        {
                            meta.Factory(&em, s_ContextEntity.GetID());
                            activeScene->MarkDirty(true);
                        }
                    }
                }
                TimeGUI::EndMenu();
            }

            if (TimeGUI::BeginMenu("+ Add Child Entity"))
            {
                if (TimeGUI::MenuItem("Empty Child Entity"))
                {
                    Entity child = activeScene->CreateEntity("Child Entity");
                    activeScene->SetParent(child, s_ContextEntity);
                    editor->ClearSelection();
                    editor->SelectEntity(child);
                    activeScene->MarkDirty(true);
                }

                TimeGUI::Separator();

                TEMap<TEString, TEArray<EntityPreset>> categorizedPresets;
                for (const auto &preset : ComponentRegistry::Get().GetEntityPresets())
                {
                    TEString cat = preset.Category.empty() ? "General" : preset.Category;
                    categorizedPresets[cat].Add(preset);
                }

                for (const auto &[catName, presets] : categorizedPresets)
                {
                    if (TimeGUI::BeginMenu(catName.c_str()))
                    {
                        for (const auto &preset : presets)
                        {
                            if (TimeGUI::MenuItem(preset.Name.c_str()))
                            {
                                Entity child = activeScene->CreateEntity(preset.Name);
                                if (preset.Create)
                                    preset.Create(child.GetID(), &em);
                                activeScene->SetParent(child, s_ContextEntity);
                                editor->ClearSelection();
                                editor->SelectEntity(child);
                                activeScene->MarkDirty(true);
                            }
                        }
                        TimeGUI::EndMenu();
                    }
                }
                TimeGUI::EndMenu();
            }

            TimeGUI::Separator();

            if (TimeGUI::MenuItem("Duplicate Entity"))
            {
                Entity dup = activeScene->CreateEntity(entityName + " (Copy)");
                auto srcComps = em.GetAllComponents(s_ContextEntity.GetID());
                for (auto *c : srcComps)
                {
                    if (c && !c->GetClassName().empty())
                    {
                        auto *meta = ComponentRegistry::Get().GetMetadata(c->GetClassName());
                        if (meta && meta->Factory)
                        {
                            auto *newComp = meta->Factory(&em, dup.GetID());
                            if (newComp)
                            {
                                newComp->Transform = c->Transform;
                                newComp->Transform.Position.x += 1.0f;
                                newComp->Transform.Position.y -= 1.0f;
                            }
                        }
                    }
                }
                editor->ClearSelection();
                editor->SelectEntity(dup);
                activeScene->MarkDirty(true);
            }

            if (TimeGUI::MenuItem("Delete Entity (Del)"))
            {
                activeScene->DestroyEntity(s_ContextEntity);
                editor->ClearSelection();
                activeScene->MarkDirty(true);
            }
        }
        TimeGUI::EndPopup();
    }

    TimeGUI::End();
}

void SceneHierarchyPanel::DrawEntityNode(EntityID entityID, EntityManager &em, Ref<EditorLayer> editor,
                                         Entity &entityToDelete)
{
    Entity entity(entityID, &em);
    TEString entityName = "Entity (" + TEString::FromInt64(static_cast<int64_t>(entityID)) + ")";
    if (auto *tag = em.GetComponent<TagComponent>(entity))
    {
        entityName = tag->Tag;
    }

    if (m_SearchBar && !m_SearchBar->Matches(entityName))
        return;

    bool isSelected = (editor->GetSelectedEntities().find(entity) != editor->GetSelectedEntities().end());

    auto *tc = em.GetComponent<TransformComponent>(entity);
    bool hasChildren = tc && !tc->Children.empty();

    TimeGUITreeNodeFlags flags = TimeGUITreeNodeFlags_OpenOnArrow | TimeGUITreeNodeFlags_SpanAvailWidth |
                                 TimeGUITreeNodeFlags_FramePadding | TimeGUITreeNodeFlags_AllowOverlap;
    if (isSelected)
        flags |= TimeGUITreeNodeFlags_Selected;
    if (!hasChildren)
        flags |= TimeGUITreeNodeFlags_Leaf;

    bool nodeOpen = TimeGUI::TreeNodeEx((void *)(uintptr_t)entityID, (int)flags, entityName);

    // Left Click = Selection
    if (TimeGUI::IsItemClicked(0))
    {
        bool multiSelect = TimeGUI::GetIO().KeyCtrl;
        editor->SelectEntity(entity, false, multiSelect);
    }
    // Right Click = Select & Open Context Menu
    if (TimeGUI::IsItemClicked(1))
    {
        editor->SelectEntity(entity, false, false);
        s_ContextEntity = entity;
        s_ShouldOpenHierarchyContextMenu = true;
    }

    // Right-aligned action buttons on the same row
    float btnSize = 22.0f;
    float spacing = 4.0f;
    TimeGUI::SameLine(TimeGUI::GetWindowWidth() - (btnSize * 2.0f + spacing + 24.0f));

    // 1. '+' Action Button (Launches Context Menu for this entity)
    TimeGUI::PushStyleColor(TimeGUICol_Button, TEVector4(0.20f, 0.25f, 0.35f, 0.70f));
    TimeGUI::PushStyleColor(TimeGUICol_ButtonHovered, TEVector4(0.24f, 0.65f, 0.35f, 0.95f));
    if (TimeGUI::Button((TEString("##AddAction_") + TEString::FromInt64(static_cast<int64_t>(entityID))).c_str(),
                        TEVector2(btnSize, btnSize)))
    {
        TE_CORE_INFO("[Hierarchy] '+' button clicked on entity '{0}' (ID: {1})", entityName, (uint64_t)entityID);
        editor->SelectEntity(entity, false, false);
        s_ContextEntity = entity;
        s_ShouldOpenHierarchyContextMenu = true;
    }
    TEVector2 addMin = TimeGUI::GetItemRectMin();
    TEVector2 addCenter = TEVector2(addMin.x + btnSize * 0.5f, addMin.y + btnSize * 0.5f);
    TimeGUIDrawList dl = TimeGUI::GetWindowDrawList();
    EditorUtils::DrawAddIcon(dl, addCenter, 14.0f, 0xFFFFFFFF);
    if (TimeGUI::IsItemHovered())
        TimeGUI::SetTooltip("Add Component / Action");
    TimeGUI::PopStyleColor(2);

    TimeGUI::SameLine(0, spacing);

    // 2. Red 'X' Delete Button
    TimeGUI::PushStyleColor(TimeGUICol_Button, TEVector4(0.65f, 0.18f, 0.18f, 0.70f));
    TimeGUI::PushStyleColor(TimeGUICol_ButtonHovered, TEVector4(0.85f, 0.22f, 0.22f, 0.95f));
    TimeGUI::PushStyleColor(TimeGUICol_ButtonActive, TEVector4(0.50f, 0.12f, 0.12f, 1.0f));
    if (TimeGUI::Button((TEString("##DelAction_") + TEString::FromInt64(static_cast<int64_t>(entityID))).c_str(),
                        TEVector2(btnSize, btnSize)))
    {
        TE_CORE_INFO("[Hierarchy] 'X' remove button clicked on entity '{0}' (ID: {1})", entityName, (uint64_t)entityID);
        entityToDelete = entity;
    }
    TEVector2 delMin = TimeGUI::GetItemRectMin();
    TEVector2 delCenter = TEVector2(delMin.x + btnSize * 0.5f, delMin.y + btnSize * 0.5f);
    EditorUtils::DrawCrossIcon(dl, delCenter, 14.0f, 0xFFFFFFFF);
    if (TimeGUI::IsItemHovered())
        TimeGUI::SetTooltip("Delete Entity");
    TimeGUI::PopStyleColor(3);

    if (nodeOpen)
    {
        if (tc)
        {
            for (EntityID childID : tc->Children)
            {
                if (em.IsValid(childID))
                {
                    DrawEntityNode(childID, em, editor, entityToDelete);
                }
            }
        }
        TimeGUI::TreePop();
    }
}

bool SceneHierarchyPanel::OnShortcut(const TEString &shortcutId, Ref<EditorLayer> editor)
{
    if (!editor || !m_Focused)
        return false;

    if (shortcutId == "Editor_Copy")
    {
        editor->CopySelectedEntities();
        return true;
    }
    if (shortcutId == "Editor_Paste")
    {
        editor->PasteSelectedEntities();
        return true;
    }
    if (shortcutId == "Editor_Duplicate")
    {
        editor->DuplicateSelectedEntities();
        return true;
    }
    if (shortcutId == "Editor_DeleteSelected")
    {
        if (!editor->GetSelectedEntities().IsEmpty())
        {
            editor->TriggerDeleteSelectedEntities();
            return true;
        }
        return false;
    }
    if (shortcutId == "Editor_ClearSelection")
    {
        editor->ClearSelection();
        return true;
    }
    return false;
}

TE_REGISTER_EDITOR_PANEL(SceneHierarchyPanel);
