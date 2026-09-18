#pragma once

#include "PreRequisites.h"
#include "EditorToolbarOverlay.hpp"

class TE_API StandaloneToolbarOverlay : public IEditorToolbarOverlay
{
public:
    void RegisterToolbarItems(Ref<EditorLayer> editor) override;
    void OnButtonClicked(const TEString &itemId, Ref<EditorLayer> editor) override;
    void OnCustomRender(const TEString &itemId, Ref<EditorLayer> editor) override;
};
