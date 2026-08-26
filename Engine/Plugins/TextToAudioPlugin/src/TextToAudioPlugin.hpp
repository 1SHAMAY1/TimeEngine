#pragma once

#include "Core/Plugin/IPlugin.hpp"

class TextToAudioPlugin : public IPlugin
{
public:
    virtual void OnLoad() override;
    virtual void OnUnload() override;

    virtual TEString GetName() const override { return "TextToAudioPlugin"; }
    virtual TEString GetVersion() const override { return "1.0.0"; }
    virtual TEString GetAuthor() const override { return "TimeEngine Team"; }
    virtual TEString GetDescription() const override
    {
        return "Offline Text-to-Audio and Speech Synthesis plugin with Acoustic RichText, GameplayTags, and MCP "
               "support.";
    }

    virtual void DrawThumbnail(TimeGUIDrawList &dl, const TEVector2 &min, const TEVector2 &max) const override;
};
