#pragma once
#include "Editor/Panels/IEditorPanel.hpp"

class TE_API EditorSettingsPanel : public IEditorPanel
{
public:
    EditorSettingsPanel() : IEditorPanel("Editor Settings") { m_Visible = false; }
    TEString GetID() const override { return "EditorSettings"; }
    TEString GetTitle() const override { return "Editor Settings"; }
    bool IsWindowMenuExposed() const override { return false; }
    void OnTimeGUIRender(Ref<EditorLayer> editor) override;

private:
    int m_SelectedIdx = 0;
};

class TE_API ProjectSettingsPanel : public IEditorPanel
{
public:
    ProjectSettingsPanel() : IEditorPanel("Project Settings") { m_Visible = false; }
    TEString GetID() const override { return "ProjectSettings"; }
    TEString GetTitle() const override { return "Project Settings"; }
    bool IsWindowMenuExposed() const override { return false; }
    void OnTimeGUIRender(Ref<EditorLayer> editor) override;

private:
    int m_SelectedIdx = 0;
};
