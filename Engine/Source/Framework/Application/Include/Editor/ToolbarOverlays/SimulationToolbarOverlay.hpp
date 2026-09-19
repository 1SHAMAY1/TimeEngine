#pragma once

#include "PreRequisites.h"
#include "EditorToolbarOverlay.hpp"

class TE_API PlayToolbarOverlay : public IEditorToolbarOverlay
{
public:
    void RegisterToolbarItems(Ref<EditorLayer> editor) override;
    void OnButtonClicked(const TEString &itemId, Ref<EditorLayer> editor) override;
};

class TE_API PauseToolbarOverlay : public IEditorToolbarOverlay
{
public:
    void RegisterToolbarItems(Ref<EditorLayer> editor) override;
    void OnButtonClicked(const TEString &itemId, Ref<EditorLayer> editor) override;
};

class TE_API StopToolbarOverlay : public IEditorToolbarOverlay
{
public:
    void RegisterToolbarItems(Ref<EditorLayer> editor) override;
    void OnButtonClicked(const TEString &itemId, Ref<EditorLayer> editor) override;
};
