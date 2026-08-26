#include "IKProjectSettings.hpp"
#include "Core/PreRequisites.h"
#include "Layers/EditorLayer.hpp"
#include "Utils/TimeGUI.hpp"

TE_REGISTER_PROJECT_SETTINGS(IKProjectSettings);

namespace
{
static int s_MaxSolverIterations = 15;
static float s_ToleranceDistance = 0.001f;
static bool s_EnableAngleConstraints = true;
static bool s_DrawDebugBones = false;
static float s_FootRaycastDistance = 1.5f;
} // namespace

void IKProjectSettings::OnDrawSettingsUI(Ref<EditorLayer> editor)
{
    TimeGUI::TextColored(TEColor(0.2f, 0.7f, 1.0f, 1.0f), "IK Solver Parameters");
    TimeGUI::Separator();

    TimeGUI::SliderInt("Max Solver Iterations", &s_MaxSolverIterations, 1, 50);
    TimeGUI::DragFloat("Convergence Tolerance", &s_ToleranceDistance, 0.0001f, 0.00001f, 0.1f, "%.5f m");
    TimeGUI::Checkbox("Enforce Joint Angle Limits", &s_EnableAngleConstraints);

    TimeGUI::Spacing();
    TimeGUI::TextColored(TEColor(0.2f, 0.7f, 1.0f, 1.0f), "Foot Grounder & Debugging");
    TimeGUI::Separator();

    TimeGUI::SliderFloat("Foot Placement Ray Distance", &s_FootRaycastDistance, 0.1f, 5.0f, "%.2f m");
    TimeGUI::Checkbox("Draw Runtime Debug Skeleton", &s_DrawDebugBones);

    TimeGUI::Spacing();
    TimeGUI::Separator();

    if (TimeGUI::Button("Reset IK Defaults", 180.0f, 26.0f))
    {
        s_MaxSolverIterations = 15;
        s_ToleranceDistance = 0.001f;
        s_EnableAngleConstraints = true;
        s_DrawDebugBones = false;
        s_FootRaycastDistance = 1.5f;
    }
}
