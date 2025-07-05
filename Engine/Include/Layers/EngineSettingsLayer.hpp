#pragma once

#include "Core/PreRequisites.h"
#include "Layers/Layer.hpp"
#include "Core/EngineSettings.hpp"
#include "imgui.h"

namespace TE {

    /**
     * EngineSettingsLayer - UI layer for managing engine settings
     * Provides a comprehensive interface for controlling all engine settings
     */
    class TE_API EngineSettingsLayer : public Layer {
    public:
        EngineSettingsLayer(const std::string& name = "Engine Settings");
        virtual ~EngineSettingsLayer();

        virtual void OnAttach() override;
        virtual void OnDetach() override;
        virtual void OnUpdate() override;
        virtual void OnImGuiRender() override;
        virtual void OnEvent(Event& event) override;

        // ===== UI Control Methods =====
        void SetVisible(bool visible) { m_IsVisible = visible; }
        bool IsVisible() const { return m_IsVisible; }
        void ToggleVisibility() { m_IsVisible = !m_IsVisible; }
        
        void SetWindowPosition(const ImVec2& pos) { m_WindowPos = pos; }
        void SetWindowSize(const ImVec2& size) { m_WindowSize = size; }
        void SetWindowTitle(const std::string& title) { m_WindowTitle = title; }

    private:
        // ===== UI Rendering Methods =====
        void RenderMainWindow();
        void RenderFrameRateTab();
        void RenderLoggingTab();
        void RenderPerformanceTab();
        void RenderDebugTab();
        void RenderWindowTab();
        void RenderRenderingTab();
        void RenderInputTab();
        void RenderAudioTab();
        void RenderFileIOTab();
        void RenderSettingsTab();

        // ===== Helper Methods =====
        void ApplyCurrentSettings();
        void ResetToDefaults();
        void LoadSettingsFromFile();
        void SaveSettingsToFile();
        void ValidateCurrentSettings();
        
        // ===== UI State =====
        bool m_IsVisible = true;
        ImVec2 m_WindowPos = ImVec2(50, 50);
        ImVec2 m_WindowSize = ImVec2(600, 500);
        std::string m_WindowTitle = "Engine Settings";
        
        // ===== Settings Cache =====
        EngineSettings& m_Settings = EngineSettings::Get();
        
        // ===== UI State Variables =====
        bool m_ShowConfirmationDialog = false;
        bool m_ShowValidationErrors = false;
        std::string m_ValidationErrorText;
        std::string m_SelectedSettingsFile = "engine_settings.json";
        
        // ===== Temporary Settings (for UI) =====
        struct TempSettings {
            // Frame Rate
            float targetFrameRate = 60.0f;
            bool unlimitedFrameRate = false;
            bool vSyncEnabled = true;
            float frameRateLimit = 60.0f;
            
            // Logging
            bool logToFile = true;
            bool logToConsole = true;
            std::string logLevel = "INFO";
            bool logTimestamp = true;
            std::string logFile = "TimeEngineLog.json";
            
            // Performance
            uint32_t maxDrawCalls = 10000;
            uint32_t maxTriangles = 1000000;
            uint32_t maxVertices = 2000000;
            uint32_t maxTextures = 1000;
            uint32_t maxShaders = 100;
            
            // Debug
            bool debugMode = false;
            bool showFPS = true;
            bool showPerformanceMetrics = true;
            bool showDebugInfo = false;
            bool showWireframe = false;
            bool showBoundingBoxes = false;
            
            // Window
            std::string windowTitle = "TimeEngine";
            uint32_t windowWidth = 1280;
            uint32_t windowHeight = 720;
            bool windowResizable = true;
            bool windowFullscreen = false;
            bool windowMaximized = false;
            
            // Rendering
            float clearColor[4] = {0.0f, 0.0f, 0.0f, 1.0f};
            bool depthTest = true;
            bool blending = true;
            bool culling = true;
            bool multisampling = false;
            
            // Input
            float mouseSensitivity = 1.0f;
            bool keyboardRepeat = true;
            bool mouseAcceleration = false;
            
            // Audio
            bool audioEnabled = true;
            float audioVolume = 1.0f;
            uint32_t audioSampleRate = 44100;
            uint32_t audioChannels = 2;
            
            // File I/O
            std::string assetPath = "assets/";
            std::string configPath = "config/";
            std::string logPath = "logs/";
            std::string savePath = "saves/";
        };
        
        TempSettings m_TempSettings;
        bool m_SettingsChanged = false;
        
        // ===== UI Colors =====
        ImVec4 m_HeaderColor = ImVec4(0.2f, 0.6f, 1.0f, 1.0f);
        ImVec4 m_WarningColor = ImVec4(1.0f, 0.5f, 0.0f, 1.0f);
        ImVec4 m_ErrorColor = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
        ImVec4 m_SuccessColor = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
    };

} // namespace TE 