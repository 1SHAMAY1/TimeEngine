#pragma once

#include "Core/Events/ApplicationEvent.h"
#include "Core/Events/KeyEvent.h"
#include "Core/Events/MouseEvent.h"
#include "Core/Scene/Scene.hpp"
#include "Editor/EditorMenubarRegistry.hpp"
#include "Editor/EditorToolbarRegistry.hpp"
#include "Editor/ViewportOverlayRegistry.hpp"
#include "GameFrameWork/GameplayUtils.hpp"
#include "Layers/Layer.hpp"

class TE_API EditorLayer : public Layer
{
public:
    EditorLayer(const TEString &startScene = "", const TEString &name = "EditorLayer");
    virtual ~EditorLayer();

    virtual void OnAttach() override;
    virtual void OnDetach() override;
    virtual void OnUpdate() override;
    virtual void OnTimeGUIRender() override;
    virtual void OnEvent(Event &event) override;

    Ref<EditorLayer> GetShared() { return std::static_pointer_cast<EditorLayer>(shared_from_this()); }

    enum class SceneState { Edit = 0, Play = 1, Pause = 2 };
    enum class GizmoType  { None = -1, Translate = 0, Rotate = 1, Scale = 2 };

    // Scene lifecycle
    void OnScenePlay();
    void OnScenePause();
    void OnSceneStop();
    SceneState GetSceneState() const { return m_SceneState; }

    TERef<Scene> GetActiveScene() const { return m_ActiveScene; }
    void SetActiveScene(TERef<Scene> scene);
    TERef<class Framebuffer> GetFramebuffer() const { return m_Framebuffer; }
    TERef<class Framebuffer> GetLightMapFramebuffer() const { return m_LightMapFramebuffer; }
    TEVector2 GetViewportPos() const { return m_ViewportPos; }
    void SetViewportPos(const TEVector2 &pos) { m_ViewportPos = pos; }
    bool IsViewportFocused() const { return m_ViewportFocused; }
    void SetViewportFocused(bool focused) { m_ViewportFocused = focused; }
    bool IsViewportHovered() const { return m_ViewportHovered; }
    void SetViewportHovered(bool hovered) { m_ViewportHovered = hovered; }
    void SetViewportSizeChanged(bool changed) { m_ViewportSizeChanged = changed; }
    Entity GetHoveredEntity() const { return m_HoveredEntity; }
    void SetHoveredEntity(Entity entity) { m_HoveredEntity = entity; }

    // Camera getters & setters
    TEVector GetCameraPosition() const { return m_CameraPosition; }
    void SetCameraPosition(const TEVector &pos) { m_CameraPosition = pos; }
    float GetCameraZoom() const { return m_CameraZoom; }
    void SetCameraZoom(float zoom) { m_CameraZoom = zoom; }

    // Editor modes
    static const TEArray<TEScope<class EditorMode>> &GetGlobalModes();
    static void SetGlobalActiveMode(const TEString &name);
    static class EditorMode *GetGlobalActiveMode();

    // Selection & Actions
    void SelectEntity(Entity entity, bool multiSelect = false, bool toggle = false);
    void ClearSelection();
    const TESet<Entity> &GetSelectedEntities() const { return m_SelectedEntities; }
    void TriggerDeleteSelectedEntities() { DeleteSelectedEntities(); }
    void DuplicateSelectedEntities();
    void CopySelectedEntities();
    void PasteSelectedEntities();

    // Universal Shortcut listener
    bool OnShortcut(const TEString &shortcutId);

    // Gizmo
    GizmoType GetGizmoType() const { return m_GizmoType; }
    void SetGizmoType(GizmoType type) { m_GizmoType = type; }
    bool IsGizmoDragging() const { return m_IsGizmoDragging; }
    void SetGizmoDragging(bool dragging) { m_IsGizmoDragging = dragging; }

    // Decentralized registration
    void RegisterToolbarItem(const struct EditorToolbarItem &item);
    void RegisterToolbarOverlay(TERef<class IEditorToolbarOverlay> overlay);
    void RegisterViewportOverlayItem(const struct ViewportOverlayItem &item);
    void RegisterViewportOverlay(TERef<class IViewportOverlay> overlay);
    void RegisterMenubarItem(const struct EditorMenubarItem &item);
    void RegisterEditorPanel(TERef<class IEditorPanel> panel);
    void PopulateOverlaysAndPanels();

    // Queries
    TEArray<struct EditorToolbarItem> GetToolbarItemsByAlignment(enum class EditorToolbarAlignment align) const;
    TEArray<struct ViewportOverlayItem> GetViewportItemsByCornerAndAlignment(enum class ViewportOverlayCorner corner, enum class ViewportOverlayAlignment align) const;
    TEArray<TERef<class IEditorPanel>> GetRegisteredPanels() const;
    TERef<class IEditorPanel> GetPanelByID(const TEString &id) const;

    template <typename T>
    TERef<T> GetPanel() const
    {
        for (auto &panel : m_Panels)
        {
            if (auto casted = std::dynamic_pointer_cast<T>(panel))
                return casted;
        }
        return nullptr;
    }

private:
    bool OnWindowClose(class WindowCloseEvent &e);
    bool OnKeyPressed(KeyPressedEvent &e);
    bool OnMouseButtonPressed(MouseButtonPressedEvent &e);
    bool OnMouseScrolled(MouseScrolledEvent &e);

    bool IsEntitySelected(Entity entity) const;
    void DeleteSelectedEntities();
    void UpdateCamera(float dt);
    void HandleViewportInput();
    void UpdateGizmoHover();
    void ProcessDeletionQueues();

    // Scene
    SceneState m_SceneState = SceneState::Edit;
    TEString m_StartScenePath;
    TERef<Scene> m_ActiveScene;
    TERef<Scene> m_EditorScene;
    TERef<Scene> m_RuntimeScene;
    TESet<Entity> m_SelectedEntities;

    // Gizmo
    GizmoType m_GizmoType = GizmoType::Translate;
    int m_GizmoOperation = -1;
    int m_HoveredGizmoAxis = -1;
    bool m_IsGizmoDragging = false;

    // Viewport
    bool m_ViewportFocused = false;
    bool m_ViewportHovered = false;
    bool m_ViewportSizeChanged = false;
    TEVector2 m_ViewportPos = {0, 0};
    Entity m_HoveredEntity;

    // Camera
    TEVector m_CameraPosition = {0.0f, 0.0f, 10.0f};
    float m_CameraZoom = 10.0f;

    // Rendering
    TERef<class Framebuffer> m_Framebuffer;
    TERef<class Framebuffer> m_LightMapFramebuffer;
    TERef<class Renderer2D> m_Renderer2D;

    // Registries (populated by overlays/panels)
    TEArray<struct EditorToolbarItem> m_ToolbarItems;
    TEArray<TERef<class IEditorToolbarOverlay>> m_ToolbarOverlayOwners;
    TEArray<struct ViewportOverlayItem> m_ViewportItems;
    TEArray<TERef<class IViewportOverlay>> m_ViewportOverlayOwners;
    TEArray<struct EditorMenubarItem> m_MenubarItems;
    TEArray<TERef<class IEditorPanel>> m_Panels;

    // Deletion queues
    TEArray<Entity> m_EntitiesToDelete;
    TEArray<std::pair<EntityID, class TComponent *>> m_ComponentsToDelete;
};

