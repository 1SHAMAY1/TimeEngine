#pragma once

#include "Core/Plugin/IPlugin.hpp"

class SoundSynthesizerPlugin : public IPlugin
{
public:
    virtual void OnLoad() override;
    virtual void OnUnload() override;

    virtual TEString GetName() const override { return "SoundSynthesizerPlugin"; }
    virtual TEString GetVersion() const override { return "1.0.0"; }
    virtual TEString GetAuthor() const override { return "TimeEngine Team"; }
    virtual TEString GetDescription() const override
    {
        return "Procedural Audio Synthesis, DSP Node Graph, Sound Baking, and Audio Studio Workspace plugin.";
    }

    virtual void DrawThumbnail(TimeGUIDrawList &dl, const TEVector2 &min, const TEVector2 &max) const override;
};
