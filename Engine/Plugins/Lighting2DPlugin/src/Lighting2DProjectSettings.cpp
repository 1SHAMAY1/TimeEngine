#include "Lighting2DProjectSettings.hpp"
#include "Core/PreRequisites.h"
#include "Layers/EditorLayer.hpp"
#include "Utils/TimeGUI.hpp"

TE_REGISTER_PROJECT_SETTINGS(Lighting2DProjectSettings);

namespace
{
static float s_AmbientColor[4] = {0.05f, 0.05f, 0.08f, 1.0f};
static float s_AmbientIntensity = 0.2f;
static int s_ShadowRaySteps = 64;
static float s_SoftShadowRadius = 2.0f;
static bool s_EnableVolumetricLight = false;
static float s_DefaultFalloffExponent = 2.0f;
} // namespace

void Lighting2DProjectSettings::OnDrawSettingsUI(Ref<EditorLayer> editor)
{
    TimeGUI::TextColored(TEColor(0.2f, 0.7f, 1.0f, 1.0f), "Global 2D Environment Lighting");
    TimeGUI::Separator();

    TimeGUI::ColorEdit4("Ambient Light Color", s_AmbientColor);
    TimeGUI::SliderFloat("Ambient Intensity", &s_AmbientIntensity, 0.0f, 2.0f, "%.2f");

    TimeGUI::Spacing();
    TimeGUI::TextColored(TEColor(0.2f, 0.7f, 1.0f, 1.0f), "Shadows & Occlusion");
    TimeGUI::Separator();

    TimeGUI::SliderInt("Shadow Ray Steps", &s_ShadowRaySteps, 16, 256);
    TimeGUI::SliderFloat("Soft Shadow Blur Radius", &s_SoftShadowRadius, 0.0f, 10.0f, "%.1f px");
    TimeGUI::Checkbox("Enable 2D Volumetric Lighting", &s_EnableVolumetricLight);
    TimeGUI::SliderFloat("Default Attenuation Exponent", &s_DefaultFalloffExponent, 1.0f, 4.0f, "%.1f");

    TimeGUI::Spacing();
    TimeGUI::Separator();

    if (TimeGUI::Button("Reset 2D Lighting Defaults", 210.0f, 26.0f))
    {
        s_AmbientColor[0] = 0.05f;
        s_AmbientColor[1] = 0.05f;
        s_AmbientColor[2] = 0.08f;
        s_AmbientColor[3] = 1.0f;
        s_AmbientIntensity = 0.2f;
        s_ShadowRaySteps = 64;
        s_SoftShadowRadius = 2.0f;
        s_EnableVolumetricLight = false;
        s_DefaultFalloffExponent = 2.0f;
    }
}
