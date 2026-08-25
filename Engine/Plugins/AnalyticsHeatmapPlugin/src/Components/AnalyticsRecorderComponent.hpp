#pragma once

#include "Core/Scene/ComponentRegistry.hpp"
#include "GameFrameWork/TComponent.hpp"
#include "GameFrameWork/GameplayUtils.hpp"
#include "Utils/TEString.hpp"

struct AnalyticsDataPoint
{
    TEVector2 Position = {0.0f, 0.0f};
    TEString EventType = "";
    float Timestamp = 0.0f;
};

class AnalyticsRecorderComponent : public TComponent
{
public:
    GENERATED_BODY(AnalyticsRecorderComponent)

    virtual TEString GetClassName() const override { return StaticClassName; }

    TEArray<AnalyticsDataPoint> SessionEvents;

    void RecordEvent(const TEString &eventType, const TEVector2 &worldPos);
    void SaveSessionToFile(const TEString &path);
};

#ifdef TE_EDITOR
T_REGISTER_COMPONENT(AnalyticsRecorderComponent, "Analytics Recorder Component")
#endif
