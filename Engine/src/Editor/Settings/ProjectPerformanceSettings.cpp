#include "Core/PreRequisites.h"
#include "Editor/Settings/ProjectPerformanceSettings.hpp"
#include "Core/Settings/GeneralEngineSettings.hpp"
#include "Layers/EditorLayer.hpp"
#include "Utils/TimeGUI.hpp"

TE_REGISTER_PROJECT_SETTINGS(ProjectPerformanceSettings);

void ProjectPerformanceSettings::OnDrawSettingsUI(Ref<EditorLayer> editor)
{
    auto &settings = GeneralEngineSettings::Get();

    TimeGUI::TextColored(TEColor(0.2f, 0.7f, 1.0f, 1.0f), "Frame Rate & Sync");
    TimeGUI::Separator();

    float targetFps = settings.GetTargetFrameRate();
    if (TimeGUI::SliderFloat("Target FPS", &targetFps, 15.0f, 360.0f, "%.0f FPS"))
    {
        settings.SetTargetFrameRate(targetFps);
    }

    bool unlimitedFps = settings.IsUnlimitedFrameRate();
    if (TimeGUI::Checkbox("Unlimited Frame Rate", &unlimitedFps))
    {
        settings.SetUnlimitedFrameRate(unlimitedFps);
    }

    bool vsync = settings.IsVSyncEnabled();
    if (TimeGUI::Checkbox("Enable VSync", &vsync))
    {
        settings.SetVSync(vsync);
    }

    float limitFps = settings.GetFrameRateLimit();
    if (TimeGUI::SliderFloat("Frame Rate Cap", &limitFps, 15.0f, 360.0f, "%.0f FPS"))
    {
        settings.SetFrameRateLimit(limitFps);
    }

    TimeGUI::Spacing();
    TimeGUI::TextColored(TEColor(0.2f, 0.7f, 1.0f, 1.0f), "Runtime Budgets & Thresholds");
    TimeGUI::Separator();

    int maxDrawCalls = (int)settings.GetMaxDrawCalls();
    if (TimeGUI::SliderInt("Max Draw Calls", &maxDrawCalls, 100, 50000))
    {
        settings.SetMaxDrawCalls((uint32_t)maxDrawCalls);
    }

    int maxTriangles = (int)settings.GetMaxTriangles();
    if (TimeGUI::SliderInt("Max Triangles", &maxTriangles, 1000, 5000000))
    {
        settings.SetMaxTriangles((uint32_t)maxTriangles);
    }

    int maxVertices = (int)settings.GetMaxVertices();
    if (TimeGUI::SliderInt("Max Vertices", &maxVertices, 1000, 10000000))
    {
        settings.SetMaxVertices((uint32_t)maxVertices);
    }

    int maxTextures = (int)settings.GetMaxTextures();
    if (TimeGUI::SliderInt("Max Loaded Textures", &maxTextures, 10, 10000))
    {
        settings.SetMaxTextures((uint32_t)maxTextures);
    }

    TimeGUI::Spacing();
    TimeGUI::TextColored(TEColor(0.2f, 0.7f, 1.0f, 1.0f), "Debug & Overlays");
    TimeGUI::Separator();

    bool showFps = settings.IsShowFPSEnabled();
    if (TimeGUI::Checkbox("Show FPS Counter", &showFps))
    {
        settings.SetShowFPS(showFps);
    }

    bool showMetrics = settings.IsShowPerformanceMetricsEnabled();
    if (TimeGUI::Checkbox("Show Performance Metrics", &showMetrics))
    {
        settings.SetShowPerformanceMetrics(showMetrics);
    }

    bool showWireframe = settings.IsShowWireframeEnabled();
    if (TimeGUI::Checkbox("Show Wireframe Overlay", &showWireframe))
    {
        settings.SetShowWireframe(showWireframe);
    }

    bool showBoxes = settings.IsShowBoundingBoxesEnabled();
    if (TimeGUI::Checkbox("Show Bounding Boxes", &showBoxes))
    {
        settings.SetShowBoundingBoxes(showBoxes);
    }

    TimeGUI::Spacing();
    TimeGUI::Separator();

    if (TimeGUI::Button("Reset Performance Defaults", 200.0f, 26.0f))
    {
        settings.SetTargetFrameRate(60.0f);
        settings.SetUnlimitedFrameRate(false);
        settings.SetVSync(true);
        settings.SetFrameRateLimit(60.0f);
        settings.SetMaxDrawCalls(10000);
        settings.SetMaxTriangles(1000000);
        settings.SetMaxVertices(2000000);
        settings.SetMaxTextures(1000);
        settings.SetShowFPS(true);
        settings.SetShowPerformanceMetrics(true);
        settings.SetShowWireframe(false);
        settings.SetShowBoundingBoxes(false);
    }
}
