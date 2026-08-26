#include "Editor/Settings/ProjectRenderingSettings.hpp"
#include "Core/PreRequisites.h"
#include "Core/Settings/GeneralEngineSettings.hpp"
#include "Layers/EditorLayer.hpp"
#include "Utils/TimeGUI.hpp"

TE_REGISTER_PROJECT_SETTINGS(ProjectRenderingSettings);

void ProjectRenderingSettings::OnDrawSettingsUI(Ref<EditorLayer> editor)
{
    auto &settings = GeneralEngineSettings::Get();

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
