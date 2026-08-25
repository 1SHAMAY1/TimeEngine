#pragma once

#include "ISubmode.hpp"
#include "../SpriteEditorTypes.hpp"


class CodeEditorSubmode : public ISubmode
{
public:
    virtual ~CodeEditorSubmode() = default;

    virtual void OnEnter(SpriteMode *mode) override;
    virtual void OnUpdate(float dt, SpriteMode *mode) override;
    virtual void OnTimeGUIRender(SpriteEditorLayer *layer, SpriteMode *mode) override;
    virtual void OnExit(SpriteMode *mode) override;
    virtual bool OnShortcut(const TEString &shortcutId, SpriteMode *mode) override;

    virtual TEString GetName() const override { return "TScript Studio"; }
    virtual TEString GetIcon() const override { return "{ }"; }

private:
    void DrawCodeEditorPanel(SpriteMode *mode);
    void DrawPreviewAndTimelinePanel(SpriteMode *mode);
    void DrawApiHelpModal(SpriteMode *mode);

private:
    bool m_ShowApiHelp = false;
    int m_SelectedTemplateIdx = 0;
    bool m_Initialized = false;
};
