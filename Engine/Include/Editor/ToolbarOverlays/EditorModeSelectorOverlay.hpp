#pragma once

#include "Core/PreRequisites.h"
#include "Editor/EditorToolbarOverlay.hpp"

class TE_API EditorModeSelectorOverlay : public IEditorToolbarOverlay
{
public:
    void RegisterToolbarItems(Ref<EditorLayer> editor) override;
    void OnCustomRender(const TEString &itemId, Ref<EditorLayer> editor) override;
};
