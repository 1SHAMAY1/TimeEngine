#pragma once
#include "Editor/ViewportOverlayRegistry.hpp"


class EditorLayer;

class TE_API GizmoOverlay : public IViewportOverlay
{
public:
    virtual ~GizmoOverlay() = default;
    virtual void RegisterViewportItem(Ref<EditorLayer> editor) = 0;
    bool IsActiveGizmo() const { return m_IsActive; }
    void SetActiveGizmo(bool active) { m_IsActive = active; }

protected:
    bool m_IsActive = false;
};

class TE_API SelectGizmoOverlay : public GizmoOverlay
{
public:
    void RegisterViewportItem(Ref<EditorLayer> editor) override;
    void OnRenderGizmo(Ref<EditorLayer> editor) override;
};

class TE_API TranslateGizmoOverlay : public GizmoOverlay
{
public:
    void RegisterViewportItem(Ref<EditorLayer> editor) override;
    void OnRenderGizmo(Ref<EditorLayer> editor) override;
    bool OnGizmoDrag(Ref<EditorLayer> editor) override;
};

class TE_API RotateGizmoOverlay : public GizmoOverlay
{
public:
    void RegisterViewportItem(Ref<EditorLayer> editor) override;
    void OnRenderGizmo(Ref<EditorLayer> editor) override;
    bool OnGizmoDrag(Ref<EditorLayer> editor) override;
};

class TE_API ScaleGizmoOverlay : public GizmoOverlay
{
public:
    void RegisterViewportItem(Ref<EditorLayer> editor) override;
    void OnRenderGizmo(Ref<EditorLayer> editor) override;
    bool OnGizmoDrag(Ref<EditorLayer> editor) override;
};

