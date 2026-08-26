#include "Editor/Panels/IEditorPanel.hpp"
#include "Core/PreRequisites.h"

static TEArray<TERef<IEditorPanel>> &GetPanelsList()
{
    static TEArray<TERef<IEditorPanel>> s_Panels;
    return s_Panels;
}

void EditorPanelRegistry::RegisterPanel(TERef<IEditorPanel> panel)
{
    if (panel)
    {
        GetPanelsList().Add(panel);
    }
}

TEArray<TERef<IEditorPanel>> EditorPanelRegistry::GetPanels() { return GetPanelsList(); }

void EditorPanelRegistry::Clear() { GetPanelsList().Clear(); }
