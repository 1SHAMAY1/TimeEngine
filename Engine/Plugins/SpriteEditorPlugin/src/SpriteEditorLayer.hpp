#pragma once

#include "Layers/Layer.hpp"
#include <memory>


class SpriteMode;

class SpriteEditorLayer : public Layer
{
public:
    SpriteEditorLayer(const TEString &name = "SpriteEditorLayer");
    virtual ~SpriteEditorLayer();

    virtual void OnAttach() override;
    virtual void OnDetach() override;
    virtual void OnUpdate() override;
    virtual void OnTimeGUIRender() override;
    virtual void OnEvent(Event &event) override;

    bool OnShortcut(const TEString &shortcutId);

    SpriteMode *GetSpriteMode() const { return m_SpriteMode.get(); }

private:
    TEScope<SpriteMode> m_SpriteMode;
};

