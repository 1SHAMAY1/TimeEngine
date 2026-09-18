#pragma once

#include "PreRequisites.h"
#include "EditorToolbarOverlay.hpp"

class TE_API EditorModeSelectorOverlay : public IEditorToolbarOverlay
{
public:
    void RegisterToolbarItems(Ref<EditorLayer> editor) override;
    void OnCustomRender(const TEString &itemId, Ref<EditorLayer> editor) override;
};
