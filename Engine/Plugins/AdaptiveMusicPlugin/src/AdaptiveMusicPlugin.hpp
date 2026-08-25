#pragma once

#include "Core/Plugin/IPlugin.hpp"

class AdaptiveMusicPlugin : public IPlugin
{
public:
    virtual void OnLoad() override;
    virtual void OnUnload() override;

    virtual TEString GetName() const override { return "AdaptiveMusicPlugin"; }
    virtual TEString GetVersion() const override { return "1.0.0"; }
    virtual TEString GetAuthor() const override { return "TimeEngine Team"; }
    virtual TEString GetDescription() const override { return "Dynamic Multi-Track Adaptive Music & Audio Stem Crossfader Plugin"; }

    virtual void DrawThumbnail(TimeGUIDrawList &dl, const TEVector2 &min, const TEVector2 &max) const override;
};
