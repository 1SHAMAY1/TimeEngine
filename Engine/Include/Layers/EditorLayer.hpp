#pragma once

#include "Layers/Layer.hpp"
#include "Core/Events/KeyEvent.h"
#include "Core/Events/MouseEvent.h"
#include "Core/Events/ApplicationEvent.h"
#include "Core/Events/ApplicationEvent.h"
#include <string>
#include "Renderer/Framebuffer.hpp"

namespace TE {

    struct EditorSettings
    {
        bool ShowPhysicsColliders = false;
        bool AllowNavigation = true;
        float CameraSpeed = 10.0f;
        float ZoomSpeed = 2.0f;
        // Theme settings can go here or remain in the dedicated function
    };

    struct ProjectSettings
    {
        enum class GameType { TwoD, ThreeD };
        enum class TwoDMode { TopDown, SideScroller };

        GameType ConfigType = GameType::TwoD;
        TwoDMode Mode2D = TwoDMode::TopDown; 
    };

    class TE_API EditorLayer : public Layer
    {
    public:
        EditorLayer(const std::string& name = "EditorLayer");
        virtual ~EditorLayer();

        virtual void OnAttach() override;
        virtual void OnDetach() override;
        virtual void OnUpdate() override;
        virtual void OnImGuiRender() override;
        virtual void OnEvent(Event& event) override;

    private:
        bool OnKeyPressed(KeyPressedEvent& e);
        bool OnMouseButtonPressed(MouseButtonPressedEvent& e);
        bool OnMouseScrolled(MouseScrolledEvent& e);
        
        // UI Helpers
        void SetDarkThemeColors();
        void UI_DrawMenubar();
        void UI_DrawToolbar();

        // Panels
        void UI_DrawSceneHierarchy();
        void UI_DrawProperties();
        void UI_DrawContentBrowser();
        void UI_DrawViewport();
        void UI_DrawSettingsPanel(); // New
        void UI_DrawProjectSettingsPanel(); // New

        // Navigation
        void UpdateCamera(float dt);

    private:
        // Layout State
        bool m_ShowSceneHierarchy = true;
        bool m_ShowProperties = true;
        bool m_ShowContentBrowser = true;
        bool m_ShowViewport = true;
        bool m_ShowSettings = false;
        bool m_ShowProjectSettings = false;

        bool m_ViewportFocused = false;
        bool m_ViewportHovered = false;
        
        // State
        EditorSettings m_EditorSettings;
        ProjectSettings m_ProjectSettings;
        
        // Camera State
        glm::vec3 m_CameraPosition = { 0.0f, 0.0f, 10.0f };
        float m_CameraZoom = 10.0f; // Ortho size or Z distance
        
        // Mock State for now (Should be moved to proper Panel classes)
        std::string m_SelectedEntity = "";
        
        // Resources
        std::shared_ptr<class Texture> m_FileIcon;
        std::shared_ptr<class Texture> m_FolderIcon; // Need this
        std::shared_ptr<class Framebuffer> m_Framebuffer;
        std::shared_ptr<class Renderer2D> m_Renderer2D;
        
        // Physics
        std::shared_ptr<class PhysicsWorld> m_PhysicsWorld;
        std::vector<struct RigidBody*> m_TestBodies;
        std::shared_ptr<class Material> m_DebugMaterial;

        bool m_ViewportSizeChanged = false; // Tracks if we need resize
        float m_LastViewportX = 0, m_LastViewportY = 0;
    };

}
