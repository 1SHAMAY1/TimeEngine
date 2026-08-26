#pragma once

#include "Core/Plugin/IPlugin.hpp"

class Skeletal2DPlugin : public IPlugin
{
public:
    virtual void OnLoad() override;
    virtual void OnUnload() override;

    virtual TEString GetName() const override { return "Skeletal2DPlugin"; }
    virtual TEString GetVersion() const override { return "1.0.0"; }
    virtual TEString GetAuthor() const override { return "TimeEngine Team"; }
    virtual TEString GetDescription() const override
    {
        return "Native 2D Skeletal Animation & Deformable Skinning Engine with Spine JSON Import, Sockets, and Editor "
               "Rigging Mode";
    }

    virtual void DrawThumbnail(TimeGUIDrawList &dl, const TEVector2 &min, const TEVector2 &max) const override;
};
