#include "AnalyticsRecorderComponent.hpp"

void AnalyticsRecorderComponent::RecordEvent(const TEString &eventType, const TEVector2 &worldPos)
{
    AnalyticsDataPoint pt;
    pt.EventType = eventType;
    pt.Position = worldPos;
    pt.Timestamp = 0.0f; // TODO: Sample engine time
    SessionEvents.Add(pt);
}

void AnalyticsRecorderComponent::SaveSessionToFile(const TEString &path)
{
    // TODO: Contributor implementation - Serialize SessionEvents to JSON or CSV
}
