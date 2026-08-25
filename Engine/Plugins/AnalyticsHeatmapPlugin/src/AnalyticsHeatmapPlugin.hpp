#pragma once

#include "Core/Plugin/IPlugin.hpp"

class AnalyticsHeatmapPlugin : public IPlugin
{
public:
    virtual void OnLoad() override;
    virtual void OnUnload() override;

    virtual TEString GetName() const override { return "AnalyticsHeatmapPlugin"; }
    virtual TEString GetVersion() const override { return "1.0.0"; }
    virtual TEString GetAuthor() const override { return "TimeEngine Team"; }
    virtual TEString GetDescription() const override { return "Player Telemetry Recording, Kernel Density Estimation & Viewport Heatmap Plugin"; }

    virtual void DrawThumbnail(TimeGUIDrawList &dl, const TEVector2 &min, const TEVector2 &max) const override;
};
