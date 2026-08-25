#pragma once
#include "Editor/Panels/IEditorPanel.hpp"
#include "Core/Scene/EntityManager.hpp"
#include <memory>


class TE_API InspectorPanel : public IEditorPanel
{
public:
    InspectorPanel();
    TEString GetID() const override { return "Inspector"; }
    TEString GetTitle() const override { return "Properties"; }
    void OnTimeGUIRender(Ref<EditorLayer> editor) override;

private:
    TERef<class UISearchBar> m_SearchBar;
    class TComponent *m_SelectedComponent = nullptr;
    Entity m_SelectedToAddComponent;
    bool m_ShouldOpenAddComponentPopup = false;
    class TComponent *m_ComponentParentForAdd = nullptr;
    class TComponent *m_RenamingComponent = nullptr;
    bool m_FocusedRenamingInput = false;
};

