#pragma once
#include "Editor/Panels/IEditorPanel.hpp"
#include "Core/Scene/EntityManager.hpp"
#include <memory>


class TE_API SceneHierarchyPanel : public IEditorPanel
{
public:
    SceneHierarchyPanel();
    TEString GetID() const override { return "SceneHierarchy"; }
    TEString GetTitle() const override { return "Scene Hierarchy"; }
    void OnTimeGUIRender(Ref<EditorLayer> editor) override;
    bool OnShortcut(const TEString &shortcutId, Ref<EditorLayer> editor) override;

private:
    void DrawEntityNode(EntityID entityID, EntityManager &em, Ref<EditorLayer> editor, Entity &entityToDelete);

    TERef<class UISearchBar> m_SearchBar;
    TERef<class UISearchBar> m_CreateModalSearchBar;
    bool m_ShowCreateEntityModal = false;
    TEString m_SelectedPresetName;
    EntityID m_RenamingEntityID = 0;
    bool m_FocusedRenamingInput = false;
    bool m_Focused = false;
    bool m_Hovered = false;
};

