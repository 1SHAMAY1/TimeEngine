#pragma once

#include "Core/PreRequisites.h"
#include "Editor/EditorToolbarOverlay.hpp"

class TE_API RestartEditorToolbarOverlay : public IEditorToolbarOverlay
{
public:
    void RegisterToolbarItems(Ref<EditorLayer> editor) override;
    void OnCustomRender(const TEString &itemId, Ref<EditorLayer> editor) override;
    static void RequestRestart();
    static void RestartEditor();
};
