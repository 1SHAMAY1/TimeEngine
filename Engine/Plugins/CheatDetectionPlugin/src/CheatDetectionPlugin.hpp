#pragma once

#include "Core/Plugin/IPlugin.hpp"

class CheatDetectionPlugin : public IPlugin
{
public:
    virtual void OnLoad() override;
    virtual void OnUnload() override;

    virtual TEString GetName() const override { return "CheatDetectionPlugin"; }
    virtual TEString GetVersion() const override { return "1.0.0"; }
    virtual TEString GetAuthor() const override { return "TimeEngine Team"; }
    virtual TEString GetDescription() const override { return "Future Multiplayer Telemetry Validation & Anomaly Detection Plugin (Architectural Stub)"; }

    virtual void DrawThumbnail(TimeGUIDrawList &dl, const TEVector2 &min, const TEVector2 &max) const override;
};
