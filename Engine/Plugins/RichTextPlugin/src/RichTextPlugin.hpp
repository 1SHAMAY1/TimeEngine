#pragma once

#include "Core/Plugin/IPlugin.hpp"


class RichTextPlugin : public IPlugin
{
public:
    virtual void OnLoad() override;
    virtual void OnUnload() override;

    virtual TEString GetName() const override { return "RichTextPlugin"; }
    virtual TEString GetVersion() const override { return "1.0.0"; }
    virtual TEString GetAuthor() const override { return "TimeEngine Team"; }
    virtual TEString GetDescription() const override
    {
        return "Dynamic Rich Text formatting, TEString markup parser, RichTextTable style sheets, "
               "real-time vertex animators, layout engine, and TimeGUI/Renderer2D integration.";
    }

    virtual void DrawThumbnail(TimeGUIDrawList &dl, const TEVector2 &min, const TEVector2 &max) const override;
};

