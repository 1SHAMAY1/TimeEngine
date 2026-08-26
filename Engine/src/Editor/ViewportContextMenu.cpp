#include "Editor/ViewportContextMenu.hpp"
#include "Core/PreRequisites.h"
#include "Core/Scene/ComponentRegistry.hpp"
#include "Core/Scene/EntityManager.hpp"
#include "Core/Scene/Scene.hpp"
#include "Editor/EditorSaveManager.hpp"
#include "Layers/EditorLayer.hpp"
#include <map>

bool ViewportContextMenu::s_IsOpen = false;
bool ViewportContextMenu::s_ShouldOpen = false;
TEVector2 ViewportContextMenu::s_SpawnPos = {0, 0};
Entity ViewportContextMenu::s_TargetEntity = Entity();

ViewportContextMenu::ViewportContextMenu(EditorLayer *editor) : Layer("ViewportContextMenu"), m_Editor(editor) {}

void ViewportContextMenu::OpenAt(const TEVector2 &screenPos, Entity hoveredEntity)
{
    s_IsOpen = true;
    s_ShouldOpen = true;
    s_SpawnPos = screenPos;
    s_TargetEntity = hoveredEntity;
    TimeGUI::OpenPopup("##ViewportFloatingContextMenu");
}

bool ViewportContextMenu::IsOpen() { return s_IsOpen; }

void ViewportContextMenu::Close()
{
    s_IsOpen = false;
    s_ShouldOpen = false;
    s_TargetEntity = Entity();
}

void ViewportContextMenu::OnTimeGUIRender()
{
    if (m_Editor)
        OnTimeGUIRender(m_Editor);
}

void ViewportContextMenu::OnTimeGUIRender(EditorLayer *editor)
{
    if (!editor || !editor->GetActiveScene())
        return;

    auto activeScene = editor->GetActiveScene();
    auto &em = activeScene->GetEntityManager();

    TimeGUI::SetNextWindowPos(s_SpawnPos, TimeGUICond_Appearing);

    TimeGUI::PushStyleVar(TimeGUIStyleVar_WindowRounding, 6.0f);
    TimeGUI::PushStyleVar(TimeGUIStyleVar_WindowPadding, TEVector2(8.0f, 8.0f));
    TimeGUI::PushStyleVar(TimeGUIStyleVar_ItemSpacing, TEVector2(6.0f, 6.0f));
    TimeGUI::PushStyleColor(TimeGUICol_PopupBg, TEVector4(0.08f, 0.09f, 0.12f, 0.98f));
    TimeGUI::PushStyleColor(TimeGUICol_Border, TEVector4(0.24f, 0.28f, 0.38f, 0.9f));

    if (s_ShouldOpen)
    {
        TimeGUI::OpenPopup("##ViewportFloatingContextMenu");
        s_ShouldOpen = false;
    }

    if (TimeGUI::BeginPopup("##ViewportFloatingContextMenu"))
    {
        bool hasEntity = s_TargetEntity.IsValid() && em.IsValid(s_TargetEntity.GetID());

        if (hasEntity)
        {
            // ── Entity Context Menu Options ──
            TEString entityName = "Entity (" + TEString::FromInt64(static_cast<int64_t>(s_TargetEntity.GetID())) + ")";
            TimeGUI::TextDisabled(entityName);
            TimeGUI::Separator();

            if (TimeGUI::MenuItem("Focus Camera (F)"))
            {
                auto comps = em.GetAllComponents(s_TargetEntity.GetID());
                for (auto *c : comps)
                {
                    if (c)
                    {
                        editor->SetCameraPosition(TEVector(c->Transform.Position.x, c->Transform.Position.y, 10.0f));
                        break;
                    }
                }
            }

            if (TimeGUI::MenuItem("Duplicate Entity"))
            {
                Entity dup = activeScene->CreateEntity(entityName + " (Copy)");
                auto srcComps = em.GetAllComponents(s_TargetEntity.GetID());
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
                                newComp->Transform.Position.x += 10.0f;
                                newComp->Transform.Position.y -= 10.0f;
                            }
                        }
                    }
                }
                editor->ClearSelection();
                editor->SelectEntity(dup);
                activeScene->MarkDirty(true);
            }

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
                            meta.Factory(&em, s_TargetEntity.GetID());
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
                    activeScene->SetParent(child, s_TargetEntity);
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
                                activeScene->SetParent(child, s_TargetEntity);
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

            if (TimeGUI::MenuItem("Delete Entity (Del)"))
            {
                activeScene->DestroyEntity(s_TargetEntity);
                editor->ClearSelection();
                activeScene->MarkDirty(true);
            }
        }
        else
        {
            // ── Empty Space Context Menu Options ──
            TimeGUI::TextDisabled("Viewport Actions");
            TimeGUI::Separator();

            if (TimeGUI::BeginMenu("+ Create Entity"))
            {
                if (TimeGUI::MenuItem("Empty Entity"))
                {
                    Entity e = activeScene->CreateEntity("Empty Entity");
                    editor->ClearSelection();
                    editor->SelectEntity(e);
                    EditorSaveManager::SaveByID("ActiveScene");
                }

                TimeGUI::Separator();

                // Group presets into categories matching the Add Entity modal
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
                                Entity e = activeScene->CreateEntity(preset.Name);
                                if (preset.Create)
                                {
                                    preset.Create(e.GetID(), &em);
                                }
                                editor->ClearSelection();
                                editor->SelectEntity(e);
                                EditorSaveManager::SaveByID("ActiveScene");
                            }
                        }
                        TimeGUI::EndMenu();
                    }
                }

                TimeGUI::EndMenu();
            }

            TimeGUI::Separator();

            if (TimeGUI::MenuItem("Reset Camera to Origin (0, 0)"))
            {
                editor->SetCameraPosition(TEVector(0.0f, 0.0f, 10.0f));
                editor->SetCameraZoom(10.0f);
            }
        }

        TimeGUI::EndPopup();
    }
    else
    {
        s_IsOpen = false;
    }

    TimeGUI::PopStyleColor(2);
    TimeGUI::PopStyleVar(3);
}
