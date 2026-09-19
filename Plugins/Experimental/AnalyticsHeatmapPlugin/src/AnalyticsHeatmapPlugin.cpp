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

#include "TestHarness.hpp"
#include "TestRegistry.hpp"

void AnalyticsHeatmapPlugin::RegisterTests()
{
    TestRegistry::RegisterTest("AnalyticsHeatmap", "GridBinningAndDensity", false,
                               [](TestContext &ctx)
                               {
                                   constexpr int GridSize = 10;
                                   int grid[GridSize][GridSize] = {0};

                                   TEArray<TEVector2> deathEvents = {
                                       {25.0f, 25.0f},
                                       {26.0f, 24.0f},
                                       {25.5f, 25.2f}, // clustered
                                       {80.0f, 80.0f}  // isolated
                                   };

                                   for (const auto &pt : deathEvents)
                                   {
                                       int gx = std::clamp(static_cast<int>(pt.x / 10.0f), 0, GridSize - 1);
                                       int gy = std::clamp(static_cast<int>(pt.y / 10.0f), 0, GridSize - 1);
                                       grid[gy][gx]++;
                                   }

                                   TE_CHECK_EQ(grid[2][2], 3);
                                   TE_CHECK_EQ(grid[8][8], 1);
                                   TE_CHECK_EQ(grid[0][0], 0);
                               });
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
