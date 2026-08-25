#pragma once

#include "Utils/TimeGUI.hpp"


class SpriteEditorLayer;
class SpriteMode;

class ISubmode
{
public:
    virtual ~ISubmode() = default;
    virtual void OnEnter(SpriteMode *mode) {}
    virtual void OnUpdate(float dt, SpriteMode *mode) {}
    virtual void OnTimeGUIRender(SpriteEditorLayer *layer, SpriteMode *mode) = 0;
    virtual void OnExit(SpriteMode *mode) {}
    virtual bool OnShortcut(const TEString &shortcutId, SpriteMode *mode) { return false; }
    virtual TEString GetName() const = 0;
    virtual TEString GetIcon() const = 0;
};

