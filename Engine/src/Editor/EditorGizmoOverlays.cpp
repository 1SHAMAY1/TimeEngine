#include "Core/PreRequisites.h"
#include "Editor/EditorGizmoOverlays.hpp"
#include "Layers/EditorLayer.hpp"

TE_REGISTER_VIEWPORT_OVERLAY(SelectGizmoOverlay);
TE_REGISTER_VIEWPORT_OVERLAY(TranslateGizmoOverlay);
TE_REGISTER_VIEWPORT_OVERLAY(RotateGizmoOverlay);
TE_REGISTER_VIEWPORT_OVERLAY(ScaleGizmoOverlay);

void SelectGizmoOverlay::RegisterViewportItem(Ref<EditorLayer> editor)
{
    ViewportOverlayItem item;
    item.id = "Gizmo_Select";
    item.label = "Select";
    item.tooltip = "Selection Mode (Q)";
    item.corner = ViewportOverlayCorner::TopLeft;
    item.alignment = ViewportOverlayAlignment::Left;
    item.priority = 0;
    item.ownerOverlay = this;
    item.isActive = [this]() { return m_IsActive; };
    item.onClick = [this, editor]()
    {
        if (editor)
        {
            editor->SetGizmoType(EditorLayer::GizmoType::None);
        }
    };
    ViewportOverlayRegistry::RegisterItem(item);
}

void SelectGizmoOverlay::OnRenderGizmo(Ref<EditorLayer> editor)
{
    // Selection outline rendering if needed
}

void TranslateGizmoOverlay::RegisterViewportItem(Ref<EditorLayer> editor)
{
    ViewportOverlayItem item;
    item.id = "Gizmo_Translate";
    item.label = "Translate";
    item.tooltip = "Translate Tool (W)";
    item.corner = ViewportOverlayCorner::TopLeft;
    item.alignment = ViewportOverlayAlignment::Left;
    item.priority = 1;
    item.ownerOverlay = this;
    item.isActive = [this]() { return m_IsActive; };
    item.onClick = [this, editor]()
    {
        if (editor)
        {
            editor->SetGizmoType(EditorLayer::GizmoType::Translate);
        }
    };
    ViewportOverlayRegistry::RegisterItem(item);
}

void TranslateGizmoOverlay::OnRenderGizmo(Ref<EditorLayer> editor)
{
    // Translate handle rendering
}

bool TranslateGizmoOverlay::OnGizmoDrag(Ref<EditorLayer> editor) { return false; }

void RotateGizmoOverlay::RegisterViewportItem(Ref<EditorLayer> editor)
{
    ViewportOverlayItem item;
    item.id = "Gizmo_Rotate";
    item.label = "Rotate";
    item.tooltip = "Rotate Tool (E)";
    item.corner = ViewportOverlayCorner::TopLeft;
    item.alignment = ViewportOverlayAlignment::Left;
    item.priority = 2;
    item.ownerOverlay = this;
    item.isActive = [this]() { return m_IsActive; };
    item.onClick = [this, editor]()
    {
        if (editor)
        {
            editor->SetGizmoType(EditorLayer::GizmoType::Rotate);
        }
    };
    ViewportOverlayRegistry::RegisterItem(item);
}

void RotateGizmoOverlay::OnRenderGizmo(Ref<EditorLayer> editor)
{
    // Rotate ring rendering
}

bool RotateGizmoOverlay::OnGizmoDrag(Ref<EditorLayer> editor) { return false; }

void ScaleGizmoOverlay::RegisterViewportItem(Ref<EditorLayer> editor)
{
    ViewportOverlayItem item;
    item.id = "Gizmo_Scale";
    item.label = "Scale";
    item.tooltip = "Scale Tool (R)";
    item.corner = ViewportOverlayCorner::TopLeft;
    item.alignment = ViewportOverlayAlignment::Left;
    item.priority = 3;
    item.ownerOverlay = this;
    item.isActive = [this]() { return m_IsActive; };
    item.onClick = [this, editor]()
    {
        if (editor)
        {
            editor->SetGizmoType(EditorLayer::GizmoType::Scale);
        }
    };
    ViewportOverlayRegistry::RegisterItem(item);
}

void ScaleGizmoOverlay::OnRenderGizmo(Ref<EditorLayer> editor)
{
    // Scale handle rendering
}

bool ScaleGizmoOverlay::OnGizmoDrag(Ref<EditorLayer> editor) { return false; }
