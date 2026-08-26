#include "Core/PreRequisites.h"
#include "Editor/Settings/EditorGizmoGridSettings.hpp"
#include "Layers/EditorLayer.hpp"
#include "Utils/TimeGUI.hpp"

TE_REGISTER_EDITOR_SETTINGS(EditorGizmoGridSettings);

namespace
{
static float s_GizmoSize = 1.0f;
static bool s_SnapTranslation = false;
static float s_SnapTranslationValue = 0.5f;
static bool s_SnapRotation = false;
static float s_SnapRotationValue = 15.0f; // degrees
static bool s_SnapScale = false;
static float s_SnapScaleValue = 0.1f;

static bool s_ShowGrid = true;
static float s_GridCellSize = 1.0f;
static int s_GridDivisions = 10;
} // namespace

void EditorGizmoGridSettings::OnDrawSettingsUI(Ref<EditorLayer> editor)
{
    TimeGUI::TextColored(TEColor(0.2f, 0.7f, 1.0f, 1.0f), "Transform Gizmo");
    TimeGUI::Separator();

    TimeGUI::SliderFloat("Gizmo Scale", &s_GizmoSize, 0.2f, 3.0f, "%.2fx");

    TimeGUI::Spacing();
    TimeGUI::TextColored(TEColor(0.2f, 0.7f, 1.0f, 1.0f), "Snapping Increments");
    TimeGUI::Separator();

    TimeGUI::Checkbox("Enable Translation Snap", &s_SnapTranslation);
    if (s_SnapTranslation)
    {
        TimeGUI::DragFloat("Translate Step", &s_SnapTranslationValue, 0.05f, 0.01f, 100.0f, "%.2f units");
    }

    TimeGUI::Checkbox("Enable Rotation Snap", &s_SnapRotation);
    if (s_SnapRotation)
    {
        TimeGUI::DragFloat("Rotate Step", &s_SnapRotationValue, 1.0f, 1.0f, 180.0f, "%.1f°");
    }

    TimeGUI::Checkbox("Enable Scale Snap", &s_SnapScale);
    if (s_SnapScale)
    {
        TimeGUI::DragFloat("Scale Step", &s_SnapScaleValue, 0.01f, 0.01f, 10.0f, "%.2fx");
    }

    TimeGUI::Spacing();
    TimeGUI::TextColored(TEColor(0.2f, 0.7f, 1.0f, 1.0f), "Viewport Grid");
    TimeGUI::Separator();

    TimeGUI::Checkbox("Show Viewport Grid", &s_ShowGrid);
    if (s_ShowGrid)
    {
        TimeGUI::DragFloat("Grid Cell Size", &s_GridCellSize, 0.1f, 0.1f, 100.0f, "%.1f units");
        TimeGUI::SliderInt("Major Grid Subdivisions", &s_GridDivisions, 2, 50);
    }

    TimeGUI::Spacing();
    TimeGUI::Separator();

    if (TimeGUI::Button("Reset Gizmo & Grid Defaults", 220.0f, 26.0f))
    {
        s_GizmoSize = 1.0f;
        s_SnapTranslation = false;
        s_SnapTranslationValue = 0.5f;
        s_SnapRotation = false;
        s_SnapRotationValue = 15.0f;
        s_SnapScale = false;
        s_SnapScaleValue = 0.1f;
        s_ShowGrid = true;
        s_GridCellSize = 1.0f;
        s_GridDivisions = 10;
    }
}
