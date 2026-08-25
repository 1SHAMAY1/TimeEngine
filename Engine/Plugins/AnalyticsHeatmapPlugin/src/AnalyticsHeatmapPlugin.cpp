#include "AnalyticsHeatmapPlugin.hpp"

TE_REGISTER_PLUGIN(AnalyticsHeatmapPlugin)

void AnalyticsHeatmapPlugin::OnLoad()
{
    // TODO: Register AnalyticsRecorderComponent and Editor Viewport Overlay
}

void AnalyticsHeatmapPlugin::OnUnload()
{
    // TODO: Cleanup overlay registrations
}

void AnalyticsHeatmapPlugin::DrawThumbnail(TimeGUIDrawList &dl, const TEVector2 &min, const TEVector2 &max) const
{
    float w = max.x - min.x;
    float h = max.y - min.y;
    TEVector2 c = TEVector2(min.x + w * 0.5f, min.y + h * 0.5f);
    dl.AddRectFilled(min, max, 0xFF141923, 6.0f);
    dl.AddRect(min, max, 0xFF3D5A80, 6.0f, 0, 1.0f);
    
    // Heatmap rings
    dl.AddCircleFilled(c, 12.0f, 0x4454A0FF);
    dl.AddCircleFilled(c, 8.0f, 0x66FFB703);
    dl.AddCircleFilled(c, 4.0f, 0xAAEF476F);
}
