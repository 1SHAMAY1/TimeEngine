#include "Core/PreRequisites.h"
#include "Editor/Settings/EditorViewportCameraSettings.hpp"
#include "Layers/EditorLayer.hpp"
#include "Utils/TimeGUI.hpp"

TE_REGISTER_EDITOR_SETTINGS(EditorViewportCameraSettings);

namespace
{
static float s_CameraMoveSpeed = 5.0f;
static float s_CameraZoomSpeed = 0.25f;
static float s_MouseSensitivity = 1.0f;
static bool s_InvertZoomDirection = false;
static bool s_SmoothPan = true;
static float s_PanDamping = 0.85f;
} // namespace

void EditorViewportCameraSettings::OnDrawSettingsUI(Ref<EditorLayer> editor)
{
    TimeGUI::TextColored(TEColor(0.2f, 0.7f, 1.0f, 1.0f), "Viewport Camera Controls");
    TimeGUI::Separator();

    TimeGUI::SliderFloat("Camera Pan Speed", &s_CameraMoveSpeed, 0.5f, 50.0f, "%.1f");
    TimeGUI::SliderFloat("Camera Zoom Speed", &s_CameraZoomSpeed, 0.05f, 2.0f, "%.2f");
    TimeGUI::SliderFloat("Mouse Sensitivity", &s_MouseSensitivity, 0.1f, 5.0f, "%.2f");

    TimeGUI::Spacing();
    TimeGUI::TextColored(TEColor(0.2f, 0.7f, 1.0f, 1.0f), "Navigation Dynamics");
    TimeGUI::Separator();

    TimeGUI::Checkbox("Invert Scroll Wheel Zoom", &s_InvertZoomDirection);
    TimeGUI::Checkbox("Enable Smooth Camera Panning", &s_SmoothPan);
    if (s_SmoothPan)
    {
        TimeGUI::SliderFloat("Pan Inertia / Damping", &s_PanDamping, 0.1f, 0.99f, "%.2f");
    }

    if (editor)
    {
        TimeGUI::Spacing();
        TimeGUI::TextColored(TEColor(0.2f, 0.7f, 1.0f, 1.0f), "Active Camera State");
        TimeGUI::Separator();

        auto camPos = editor->GetCameraPosition();
        float posArr[3] = {camPos.x, camPos.y, camPos.z};
        if (TimeGUI::DragFloat3("Position (X, Y, Z)", posArr, 0.1f))
        {
            editor->SetCameraPosition(TEVector(posArr[0], posArr[1], posArr[2]));
        }

        float zoom = editor->GetCameraZoom();
        if (TimeGUI::SliderFloat("Zoom Level", &zoom, 0.01f, 20.0f, "%.2fx"))
        {
            editor->SetCameraZoom(zoom);
        }
    }

    TimeGUI::Spacing();
    TimeGUI::Separator();

    if (TimeGUI::Button("Reset Camera Defaults", 200.0f, 26.0f))
    {
        s_CameraMoveSpeed = 5.0f;
        s_CameraZoomSpeed = 0.25f;
        s_MouseSensitivity = 1.0f;
        s_InvertZoomDirection = false;
        s_SmoothPan = true;
        s_PanDamping = 0.85f;
    }
}
