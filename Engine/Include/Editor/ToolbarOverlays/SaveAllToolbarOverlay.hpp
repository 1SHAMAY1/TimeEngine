#pragma once

#include "Core/PreRequisites.h"
#include "Editor/EditorToolbarOverlay.hpp"

class TE_API SaveAllToolbarOverlay : public IEditorToolbarOverlay
{
public:
    void RegisterToolbarItems(Ref<EditorLayer> editor) override;
    void OnCustomRender(const TEString &itemId, Ref<EditorLayer> editor) override;
    bool OnShortcut(const TEString &shortcutId, Ref<EditorLayer> editor) override;
    static void OpenSaveModal(bool isAppExit = false);
    static void OpenSaveModalWithAction(std::function<void()> onProceed);
};
