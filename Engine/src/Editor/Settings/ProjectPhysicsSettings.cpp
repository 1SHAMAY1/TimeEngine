#include "Editor/Settings/ProjectPhysicsSettings.hpp"
#include "Core/PreRequisites.h"
#include "Layers/EditorLayer.hpp"
#include "Utils/TimeGUI.hpp"

TE_REGISTER_PROJECT_SETTINGS(ProjectPhysicsSettings);

namespace
{
static float s_Gravity[2] = {0.0f, -9.81f};
static int s_VelocityIterations = 6;
static int s_PositionIterations = 2;
static bool s_AllowSleep = true;
static bool s_ContinuousCollision = false;
static float s_DefaultFriction = 0.3f;
static float s_DefaultRestitution = 0.0f;
} // namespace

void ProjectPhysicsSettings::OnDrawSettingsUI(Ref<EditorLayer> editor)
{
    TimeGUI::TextColored(TEColor(0.2f, 0.7f, 1.0f, 1.0f), "2D Simulation Defaults");
    TimeGUI::Separator();

    TimeGUI::DragFloat2("Gravity (X, Y)", s_Gravity, 0.1f, -100.0f, 100.0f, "%.2f m/s²");
    TimeGUI::SliderInt("Velocity Iterations", &s_VelocityIterations, 1, 30);
    TimeGUI::SliderInt("Position Iterations", &s_PositionIterations, 1, 30);
    TimeGUI::Checkbox("Allow Bodies to Sleep", &s_AllowSleep);
    TimeGUI::Checkbox("Continuous Collision Detection (CCD)", &s_ContinuousCollision);

    TimeGUI::Spacing();
    TimeGUI::TextColored(TEColor(0.2f, 0.7f, 1.0f, 1.0f), "Default Physical Material");
    TimeGUI::Separator();

    TimeGUI::SliderFloat("Default Friction", &s_DefaultFriction, 0.0f, 1.0f, "%.2f");
    TimeGUI::SliderFloat("Default Restitution (Bounciness)", &s_DefaultRestitution, 0.0f, 1.0f, "%.2f");

    TimeGUI::Spacing();
    TimeGUI::Separator();

    if (TimeGUI::Button("Reset to Physics Defaults", 180.0f, 26.0f))
    {
        s_Gravity[0] = 0.0f;
        s_Gravity[1] = -9.81f;
        s_VelocityIterations = 6;
        s_PositionIterations = 2;
        s_AllowSleep = true;
        s_ContinuousCollision = false;
        s_DefaultFriction = 0.3f;
        s_DefaultRestitution = 0.0f;
    }
}
