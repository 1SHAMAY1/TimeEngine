#pragma once
#include "Editor/Panels/IEditorPanel.hpp"


class TE_API ViewportPanel : public IEditorPanel
{
public:
    ViewportPanel() : IEditorPanel("Viewport") {}
    TEString GetID() const override { return "Viewport"; }
    TEString GetTitle() const override { return "Viewport"; }
    bool IsWindowMenuExposed() const override { return false; }
    void OnTimeGUIRender(Ref<EditorLayer> editor) override;
    bool OnShortcut(const TEString &shortcutId, Ref<EditorLayer> editor) override;
};

