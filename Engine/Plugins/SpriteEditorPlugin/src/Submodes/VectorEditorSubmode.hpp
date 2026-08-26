#pragma once

#include "../SpriteEditorTypes.hpp"
#include "ISubmode.hpp"
#include <vector>

class VectorEditorSubmode : public ISubmode
{
public:
    virtual ~VectorEditorSubmode() = default;

    virtual void OnEnter(SpriteMode *mode) override;
    virtual void OnUpdate(float dt, SpriteMode *mode) override;
    virtual void OnTimeGUIRender(SpriteEditorLayer *layer, SpriteMode *mode) override;
    virtual void OnExit(SpriteMode *mode) override;
    virtual bool OnShortcut(const TEString &shortcutId, SpriteMode *mode) override;

    virtual TEString GetName() const override { return "Vector Editor"; }
    virtual TEString GetIcon() const override { return "V"; }

private:
    void DrawToolSidebar(SpriteMode *mode);
    void DrawVectorCanvas(SpriteMode *mode);
    void DrawPropertiesSidebar(SpriteMode *mode);
};
