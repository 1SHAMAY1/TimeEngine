#pragma once

#include "../SpriteEditorTypes.hpp"
#include "ISubmode.hpp"

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
    void DrawAnimationFrameStrip(SpriteMode *mode);
    void DrawVectorCanvas(SpriteMode *mode);
    void DrawPropertiesSidebar(SpriteMode *mode);

    // Interactive Drawing State
    bool m_IsInteracting = false;
    TEVector2 m_StartNormPos = TEVector2(0, 0);
    VectorElement m_LivePreviewElem;
    TEArray<TEVector2> m_PenCurrentPath;

    // Animation / Playback
    bool m_IsPlayingPreview = true;
    int m_PreviewFPS = 8;
    float m_AnimationTimer = 0.0f;
    int m_PreviewFrameIndex = 0;

    // Vector Onion Skinning State
    bool m_EnableOnionSkin = true;
    int m_OnionPastFrames = 1;
    int m_OnionFutureFrames = 1;
    float m_OnionOpacity = 0.35f;
};
