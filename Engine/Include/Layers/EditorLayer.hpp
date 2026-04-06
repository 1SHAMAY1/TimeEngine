#pragma once

#include "Core/Events/ApplicationEvent.h"
#include "Core/Events/KeyEvent.h"
#include "Core/Events/MouseEvent.h"
#include "Core/Scene/Scene.hpp"
#include "Layers/Layer.hpp"
#include "Renderer/Framebuffer.hpp"
#include <filesystem>
#include <glm/glm.hpp>
#include <memory>
#include <set>
#include <string>
#include <vector>

namespace TE
{

struct EditorSettings
{
    bool ShowPhysicsColliders = false;
    bool AllowNavigation = true;
    float SpeedMultiplier = 1.0f; // Slider 0.1 to 100
    float BaseCameraSpeed = 100.0f;
    float ZoomSpeed = 2.0f;
    float DefaultZoom = 10.0f;
    std::map<std::string, KeyCode> Shortcuts;
};

struct ProjectSettings
{
    enum class GameType
    {
        TwoD,
        ThreeD
    };
    enum class TwoDMode
    {
        TopDown,
        SideScroller
    };

    GameType ConfigType = GameType::TwoD;
    TwoDMode Mode2D = TwoDMode::TopDown;
};

class TE_API EditorLayer : public Layer
{
public:
    EditorLayer(const std::string &name = "EditorLayer");
    virtual ~EditorLayer();

    virtual void OnAttach() override;
    virtual void OnDetach() override;
    virtual void OnUpdate() override;
    virtual void OnImGuiRender() override;
    virtual void OnEvent(Event &event) override;

private:
    bool OnKeyPressed(KeyPressedEvent &e);
    bool OnMouseButtonPressed(MouseButtonPressedEvent &e);
    bool OnMouseScrolled(MouseScrolledEvent &e);

    // Selection Helpers
    bool IsEntitySelected(Entity entity) const;
    void SelectEntity(Entity entity, bool multiSelect = false, bool toggle = false);
    void SelectComponent(class TComponent *component);
    void ClearSelection();
    void DeleteSelectedEntities();

    // Save Helpers
    void SaveScene();
    void SaveProject();
    void LoadScene(const std::filesystem::path &filepath);
    void UI_DrawSaveScenePopup();

    // Gizmo Helpers
    void UI_DrawGizmos();

    // UI Helpers
    void SetDarkThemeColors();
    void UI_DrawMenubar();
    void UI_DrawToolbar();

    // Panels
    void UI_DrawSceneHierarchy();
    void UI_DrawProperties();
    void UI_DrawContentBrowser();
    void UI_DrawViewport();
    void UI_DrawSettingsPanel();
    void UI_DrawProjectSettingsPanel();
    void UI_DrawGizmoText();
    void UI_ViewportContextMenu();
    void DrawComponentNode(Entity entity, class TComponent *comp);
    std::string GetKeyName(KeyCode key);
    int ToImGuiKey(KeyCode key);

    // Navigation
    void UpdateCamera(float dt);
    void HandleViewportInput();
    void UpdateGizmoHover();

    // Layout State
    bool m_ShowSceneHierarchy = true;
    bool m_ShowProperties = true;
    bool m_ShowContentBrowser = true;
    bool m_ShowViewport = true;
    bool m_ShowSettings = false;
    bool m_ShowProjectSettings = false;
    bool m_ShowSaveScenePopup = false;
    bool m_SaveSceneAs = false;
    char m_SaveSceneNameBuffer[256] = "";
    char m_SaveScenePathBuffer[256] = "";

    std::filesystem::path m_ContentBrowserCurrentDirectory;
    char m_ContentBrowserPathBuffer[512] = "";

    bool m_ViewportFocused = false;
    bool m_ViewportHovered = false;

    // State
    EditorSettings m_EditorSettings;
    ProjectSettings m_ProjectSettings;

    // Camera State
    glm::vec3 m_CameraPosition = {0.0f, 0.0f, 10.0f};
    float m_CameraZoom = 10.0f; // Ortho size or Z distance

    // Scene State
    std::shared_ptr<Scene> m_ActiveScene;
    std::set<Entity> m_SelectedEntities;
    Entity m_SelectedToAddComponent;
    bool m_ShouldOpenAddComponentPopup = false;
    class TComponent *m_ComponentParentForAdd = nullptr; // Parent component for the new component
    class TComponent *m_SelectedComponent = nullptr;

    // Renaming state
    EntityID m_RenamingEntityID = 0;
    class TComponent *m_RenamingComponent = nullptr;
    bool m_FocusedRenamingInput = false;

    // Gizmo State
    enum class GizmoType
    {
        None = -1,
        Translate = 0,
        Rotate = 1,
        Scale = 2
    };
    GizmoType m_GizmoType = GizmoType::Translate;
    int m_GizmoOperation = -1;   // -1: none, 0: X, 1: Y, 2: Z/Center
    int m_HoveredGizmoAxis = -1; // Cached for interaction check

    // Resources
    std::shared_ptr<class Texture> m_FileIcon;
    std::shared_ptr<class Texture> m_FolderIcon;
    std::shared_ptr<class Texture> m_LeftArrowIcon;
    std::shared_ptr<class Texture> m_SaveIcon;
    std::shared_ptr<class Texture> m_PlayIcon;
    std::shared_ptr<class Texture> m_BrandingIcon;
    std::shared_ptr<class Framebuffer> m_Framebuffer;
    std::shared_ptr<class Framebuffer> m_LightMapFramebuffer;
    std::shared_ptr<class Renderer2D> m_Renderer2D;

    // Physics
    std::shared_ptr<class PhysicsWorld> m_PhysicsWorld;
    std::vector<struct RigidBody *> m_TestBodies;
    std::shared_ptr<class Material> m_DebugMaterial;
    std::shared_ptr<class Material> m_LightBlendMaterial;
    std::shared_ptr<class Material> m_GizmoXMaterial;
    std::shared_ptr<class Material> m_GizmoYMaterial;
    std::shared_ptr<class Material> m_GizmoMaterial;

    bool m_ViewportSizeChanged = false; // Tracks if we need resize
    float m_LastViewportX = 0, m_LastViewportY = 0;
    glm::vec2 m_ViewportPos = {0, 0};
    Entity m_HoveredEntity; // For context menu

    // Gizmo Dragging
    glm::vec2 m_GizmoDragStartMousePos = {0.0f, 0.0f};
    glm::vec3 m_GizmoDragStartEntityPos = {0.0f, 0.0f, 0.0f};
    glm::vec3 m_GizmoDragStartEntityScale = {1.0f, 1.0f, 1.0f};
    float m_GizmoDragStartEntityRotation = 0.0f;

    // Save Display
    float m_SaveMessageTimer = 0.0f;

    // Deletion Queues
    std::vector<Entity> m_EntitiesToDelete;
    std::vector<std::pair<EntityID, class TComponent *>> m_ComponentsToDelete;
    void ProcessDeletionQueues();
};

} // namespace TE
