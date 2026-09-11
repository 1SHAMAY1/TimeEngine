#include "Core/PreRequisites.h"
#include "Editor/Settings/ProjectRenderingSettings.hpp"
#include "Core/Project/Project.hpp"
#include "Core/Settings/GeneralEngineSettings.hpp"
#include "Core/Packaging/GamePackager.hpp"
#include "Layers/EditorLayer.hpp"
#include "Renderer/RendererContext.hpp"
#include "Renderer/GraphicsAPI.hpp"
#include "Utils/TimeGUI.hpp"

TE_REGISTER_PROJECT_SETTINGS(ProjectRenderingSettings, true);

void ProjectRenderingSettings::OnDrawSettingsUI(Ref<EditorLayer> editor)
{
    auto &settings = GeneralEngineSettings::Get();

#if !defined(TE_PLATFORM_MACOS)
    TimeGUI::TextColored(TEColor(0.2f, 0.7f, 1.0f, 1.0f), "Graphics Backend & Hardware");
    TimeGUI::Separator();
    TimeGUI::Spacing();

#if defined(TE_PLATFORM_WINDOWS)
    static const GraphicsAPI s_AvailableAPIs[] = {GraphicsAPI::OpenGL, GraphicsAPI::DirectX11, GraphicsAPI::Vulkan,
                                                  GraphicsAPI::OpenGLES};
#elif defined(TE_PLATFORM_LINUX)
    static const GraphicsAPI s_AvailableAPIs[] = {GraphicsAPI::OpenGL, GraphicsAPI::Vulkan, GraphicsAPI::OpenGLES};
#endif
    constexpr int s_APICount = sizeof(s_AvailableAPIs) / sizeof(s_AvailableAPIs[0]);

    GraphicsAPI currentAPI = RendererContext::GetAPI();
    int currentIdx = 0;
    const char *apiNames[s_APICount];
    static TEString s_CachedNames[s_APICount];

    for (int i = 0; i < s_APICount; ++i)
    {
        s_CachedNames[i] = GamePackager::GetRendererDisplayName(s_AvailableAPIs[i]);
        apiNames[i] = s_CachedNames[i].c_str();

        if (s_AvailableAPIs[i] == currentAPI)
            currentIdx = i;
    }

    TimeGUI::PushItemWidth(280.0f);
    if (TimeGUI::Combo("Active Graphics API", &currentIdx, apiNames, s_APICount))
    {
        if (currentIdx >= 0 && currentIdx < s_APICount)
        {
            RendererContext::SetAPI(s_AvailableAPIs[currentIdx]);
            Project::SaveProjectSettings();
            settings.SaveToFile(settings.GetConfigPath() + "EngineSettings.ini");
        }
    }
    TimeGUI::PopItemWidth();

    TimeGUI::Spacing();
    TimeGUI::TextDisabled("Requires application restart to initialize the selected Graphics Context.");
    TimeGUI::Spacing();
#endif

    // Hardware Telemetry
    TEString gpuVendor = RendererContext::GetGPUVendor();
    TEString gpuRenderer = RendererContext::GetGPURenderer();
    TEString gpuType = RendererContext::GetGPUType();

    TimeGUI::TextColored(TEColor(0.7f, 0.85f, 1.0f, 1.0f), "Active GPU Hardware:");
    TimeGUI::Text(TEString("  - Vendor:   ") + (gpuVendor.IsEmpty() ? "Auto-detected" : gpuVendor));
    TimeGUI::Text(TEString("  - Device:   ") + (gpuRenderer.IsEmpty() ? "Primary Adapter" : gpuRenderer));
    TimeGUI::Text(TEString("  - GPU Type: ") + gpuType);

    TimeGUI::Spacing();
    TimeGUI::Separator();
    TimeGUI::Spacing();

    TimeGUI::TextColored(TEColor(0.2f, 0.7f, 1.0f, 1.0f), "Default Viewport & Display Pipeline");
    TimeGUI::Separator();

    float clearColor[4];
    const float *rawClear = settings.GetClearColor();
    clearColor[0] = rawClear[0];
    clearColor[1] = rawClear[1];
    clearColor[2] = rawClear[2];
    clearColor[3] = rawClear[3];

    if (TimeGUI::ColorEdit4("Clear Color", clearColor))
    {
        settings.SetClearColor(clearColor[0], clearColor[1], clearColor[2], clearColor[3]);
    }

    TimeGUI::Spacing();
    TimeGUI::TextColored(TEColor(0.2f, 0.7f, 1.0f, 1.0f), "Pipeline Stages");
    TimeGUI::Separator();

    bool depthTest = settings.IsDepthTestEnabled();
    if (TimeGUI::Checkbox("Depth Test", &depthTest))
    {
        settings.SetDepthTest(depthTest);
    }

    bool blending = settings.IsBlendingEnabled();
    if (TimeGUI::Checkbox("Alpha Blending", &blending))
    {
        settings.SetBlending(blending);
    }

    bool culling = settings.IsCullingEnabled();
    if (TimeGUI::Checkbox("Backface Culling", &culling))
    {
        settings.SetCulling(culling);
    }

    bool multisampling = settings.IsMultisamplingEnabled();
    if (TimeGUI::Checkbox("Multisampling (MSAA)", &multisampling))
    {
        settings.SetMultisampling(multisampling);
    }

    TimeGUI::Spacing();
    TimeGUI::Separator();

    if (TimeGUI::Button("Reset Rendering Defaults", 180.0f, 26.0f))
    {
        settings.SetClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        settings.SetDepthTest(true);
        settings.SetBlending(true);
        settings.SetCulling(true);
        settings.SetMultisampling(false);
    }
}
