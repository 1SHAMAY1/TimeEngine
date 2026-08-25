#pragma once
#include "Editor/Panels/IEditorPanel.hpp"
#include <memory>


class TE_API PluginsPanel : public IEditorPanel
{
public:
    PluginsPanel();
    TEString GetID() const override { return "Plugins"; }
    TEString GetTitle() const override { return "Plugins"; }
    bool IsWindowMenuExposed() const override { return false; }
    void OnTimeGUIRender(Ref<EditorLayer> editor) override;

private:
    TERef<class UISearchBar> m_SearchBar;
};

