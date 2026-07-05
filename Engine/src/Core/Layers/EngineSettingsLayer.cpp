#include "Utils/TimeGUI.hpp"
#include "Layers/EngineSettingsLayer.hpp"
#include "Core/Log.h"
#include "Core/Application.h"
#include "Utils/TimeGUI.hpp"
#include <sstream>
#include <iomanip>

namespace TE {

    EngineSettingsLayer::EngineSettingsLayer(const std::string& name)
        : Layer(name) {
        TE_CORE_INFO("EngineSettingsLayer created");
    }

    EngineSettingsLayer::~EngineSettingsLayer() {
        TE_CORE_INFO("EngineSettingsLayer destroyed");
    }

    void EngineSettingsLayer::OnAttach() {
        TE_CORE_INFO("EngineSettingsLayer attached");
        
        // Initialize temporary settings with current engine settings
        m_TempSettings.targetFrameRate = m_Settings.GetTargetFrameRate();
        m_TempSettings.unlimitedFrameRate = m_Settings.IsUnlimitedFrameRate();
        m_TempSettings.vSyncEnabled = m_Settings.IsVSyncEnabled();
        m_TempSettings.frameRateLimit = m_Settings.GetFrameRateLimit();
        
        m_TempSettings.logToFile = m_Settings.IsLogToFileEnabled();
        m_TempSettings.logToConsole = m_Settings.IsLogToConsoleEnabled();
        m_TempSettings.logLevel = m_Settings.GetLogLevel();
        m_TempSettings.logTimestamp = m_Settings.IsLogTimestampEnabled();
        m_TempSettings.logFile = m_Settings.GetLogFile();
        
        m_TempSettings.maxDrawCalls = m_Settings.GetMaxDrawCalls();
        m_TempSettings.maxTriangles = m_Settings.GetMaxTriangles();
        m_TempSettings.maxVertices = m_Settings.GetMaxVertices();
        m_TempSettings.maxTextures = m_Settings.GetMaxTextures();
        m_TempSettings.maxShaders = m_Settings.GetMaxShaders();
        
        m_TempSettings.debugMode = m_Settings.IsDebugModeEnabled();
        m_TempSettings.showFPS = m_Settings.IsShowFPSEnabled();
        m_TempSettings.showPerformanceMetrics = m_Settings.IsShowPerformanceMetricsEnabled();
        m_TempSettings.showDebugInfo = m_Settings.IsShowDebugInfoEnabled();
        m_TempSettings.showWireframe = m_Settings.IsShowWireframeEnabled();
        m_TempSettings.showBoundingBoxes = m_Settings.IsShowBoundingBoxesEnabled();
        
        m_TempSettings.windowTitle = m_Settings.GetWindowTitle();
        m_TempSettings.windowWidth = m_Settings.GetWindowWidth();
        m_TempSettings.windowHeight = m_Settings.GetWindowHeight();
        m_TempSettings.windowResizable = m_Settings.IsWindowResizable();
        m_TempSettings.windowFullscreen = m_Settings.IsWindowFullscreen();
        m_TempSettings.windowMaximized = m_Settings.IsWindowMaximized();
        
        const float* clearColor = m_Settings.GetClearColor();
        m_TempSettings.clearColor[0] = clearColor[0];
        m_TempSettings.clearColor[1] = clearColor[1];
        m_TempSettings.clearColor[2] = clearColor[2];
        m_TempSettings.clearColor[3] = clearColor[3];
        m_TempSettings.depthTest = m_Settings.IsDepthTestEnabled();
        m_TempSettings.blending = m_Settings.IsBlendingEnabled();
        m_TempSettings.culling = m_Settings.IsCullingEnabled();
        m_TempSettings.multisampling = m_Settings.IsMultisamplingEnabled();
        
        m_TempSettings.mouseSensitivity = m_Settings.GetMouseSensitivity();
        m_TempSettings.keyboardRepeat = m_Settings.IsKeyboardRepeatEnabled();
        m_TempSettings.mouseAcceleration = m_Settings.IsMouseAccelerationEnabled();
        
        m_TempSettings.audioEnabled = m_Settings.IsAudioEnabled();
        m_TempSettings.audioVolume = m_Settings.GetAudioVolume();
        m_TempSettings.audioSampleRate = m_Settings.GetAudioSampleRate();
        m_TempSettings.audioChannels = m_Settings.GetAudioChannels();
        
        m_TempSettings.assetPath = m_Settings.GetAssetPath();
        m_TempSettings.configPath = m_Settings.GetConfigPath();
        m_TempSettings.logPath = m_Settings.GetLogPath();
        m_TempSettings.savePath = m_Settings.GetSavePath();
    }

    void EngineSettingsLayer::OnDetach() {
        TE_CORE_INFO("EngineSettingsLayer detached");
    }

    void EngineSettingsLayer::OnUpdate() {
        // Update logic if needed
    }

    void EngineSettingsLayer::OnTimeGUIRender() {
        if (!m_IsVisible) return;
        
        RenderMainWindow();
        
        // Render confirmation dialog if needed
        if (m_ShowConfirmationDialog) {
            TimeGUI::OpenPopup("Confirm Reset");
            if (TimeGUI::BeginPopupModal("Confirm Reset", nullptr, TimeGUIWindowFlags_AlwaysAutoResize)) {
                TimeGUI::Text("Are you sure you want to reset all settings to defaults?");
                TimeGUI::Text("This action cannot be undone.");
                TimeGUI::Separator();
                
                if (TimeGUI::Button("Yes", TEVector2(120, 0))) {
                    ResetToDefaults();
                    m_ShowConfirmationDialog = false;
                    TimeGUI::CloseCurrentPopup();
                }
                TimeGUI::SetItemDefaultFocus();
                TimeGUI::SameLine();
                if (TimeGUI::Button("No", TEVector2(120, 0))) {
                    m_ShowConfirmationDialog = false;
                    TimeGUI::CloseCurrentPopup();
                }
                TimeGUI::EndPopup();
            }
        }
        
        // Render validation errors if needed
        if (m_ShowValidationErrors) {
            TimeGUI::OpenPopup("Validation Errors");
            if (TimeGUI::BeginPopupModal("Validation Errors", nullptr, TimeGUIWindowFlags_AlwaysAutoResize)) {
                TimeGUI::TextColored(m_ErrorColor, "The following validation errors were found:");
                TimeGUI::Separator();
                TimeGUI::TextWrapped("%s", m_ValidationErrorText.c_str());
                TimeGUI::Separator();
                
                if (TimeGUI::Button("OK", TEVector2(120, 0))) {
                    m_ShowValidationErrors = false;
                    TimeGUI::CloseCurrentPopup();
                }
                TimeGUI::EndPopup();
            }
        }
    }

    void EngineSettingsLayer::OnEvent(Event& event) {
        // Handle events if needed
    }

    void EngineSettingsLayer::RenderMainWindow() {
        TimeGUI::SetNextWindowPos(m_WindowPos, TimeGUICond_FirstUseEver);
        TimeGUI::SetNextWindowSize(m_WindowSize, TimeGUICond_FirstUseEver);
        
        TimeGUIWindowFlags windowFlags = TimeGUIWindowFlags_NoCollapse;
        
        bool windowOpen = true;
        if (TimeGUI::Begin(m_WindowTitle.c_str(), &windowOpen, windowFlags)) {
            if (TimeGUI::BeginTabBar("SettingsTabs")) {
                if (TimeGUI::BeginTabItem("Frame Rate")) {
                    RenderFrameRateTab();
                    TimeGUI::EndTabItem();
                }
                if (TimeGUI::BeginTabItem("Logging")) {
                    RenderLoggingTab();
                    TimeGUI::EndTabItem();
                }
                if (TimeGUI::BeginTabItem("Performance")) {
                    RenderPerformanceTab();
                    TimeGUI::EndTabItem();
                }
                if (TimeGUI::BeginTabItem("Debug")) {
                    RenderDebugTab();
                    TimeGUI::EndTabItem();
                }
                if (TimeGUI::BeginTabItem("Window")) {
                    RenderWindowTab();
                    TimeGUI::EndTabItem();
                }
                if (TimeGUI::BeginTabItem("Rendering")) {
                    RenderRenderingTab();
                    TimeGUI::EndTabItem();
                }
                if (TimeGUI::BeginTabItem("Input")) {
                    RenderInputTab();
                    TimeGUI::EndTabItem();
                }
                if (TimeGUI::BeginTabItem("Audio")) {
                    RenderAudioTab();
                    TimeGUI::EndTabItem();
                }
                if (TimeGUI::BeginTabItem("File I/O")) {
                    RenderFileIOTab();
                    TimeGUI::EndTabItem();
                }
                if (TimeGUI::BeginTabItem("Settings")) {
                    RenderSettingsTab();
                    TimeGUI::EndTabItem();
                }
                TimeGUI::EndTabBar();
            }
        }
        TimeGUI::End();
        
        if (!windowOpen) {
            m_IsVisible = false;
        }
    }

    void EngineSettingsLayer::RenderFrameRateTab() {
        TimeGUI::TextColored(m_HeaderColor, "Frame Rate Settings");
        TimeGUI::Separator();
        
        bool changed = false;
        
        // Target Frame Rate
        changed |= TimeGUI::SliderFloat("Target Frame Rate", &m_TempSettings.targetFrameRate, 1.0f, 300.0f, "%.1f FPS");
        
        // Unlimited Frame Rate
        changed |= TimeGUI::Checkbox("Unlimited Frame Rate", &m_TempSettings.unlimitedFrameRate);
        
        // VSync
        changed |= TimeGUI::Checkbox("VSync", &m_TempSettings.vSyncEnabled);
        
        // Frame Rate Limit
        changed |= TimeGUI::SliderFloat("Frame Rate Limit", &m_TempSettings.frameRateLimit, 1.0f, 300.0f, "%.1f FPS");
        
        if (changed) {
            m_SettingsChanged = true;
        }
        
        TimeGUI::Separator();
        TimeGUI::Text("Current Settings:");
        TimeGUI::Text("  Target FPS: %.1f", m_Settings.GetTargetFrameRate());
        TimeGUI::Text("  Unlimited: %s", m_Settings.IsUnlimitedFrameRate() ? "Yes" : "No");
        TimeGUI::Text("  VSync: %s", m_Settings.IsVSyncEnabled() ? "Enabled" : "Disabled");
        TimeGUI::Text("  Limit: %.1f FPS", m_Settings.GetFrameRateLimit());
    }

    void EngineSettingsLayer::RenderLoggingTab() {
        TimeGUI::TextColored(m_HeaderColor, "Logging Settings");
        TimeGUI::Separator();
        
        bool changed = false;
        
        // Log Output
        changed |= TimeGUI::Checkbox("Log to File", &m_TempSettings.logToFile);
        changed |= TimeGUI::Checkbox("Log to Console", &m_TempSettings.logToConsole);
        changed |= TimeGUI::Checkbox("Include Timestamps", &m_TempSettings.logTimestamp);
        
        // Log Level
        const char* logLevels[] = {"DEBUG", "INFO", "WARNING", "ERROR", "CRITICAL"};
        static int currentLogLevel = 1; // Default to INFO
        if (TimeGUI::Combo("Log Level", &currentLogLevel, logLevels, (int)(sizeof(logLevels) / sizeof(logLevels[0])))) {
            m_TempSettings.logLevel = logLevels[currentLogLevel];
            changed = true;
        }
        
        // Log File
        char logFileBuffer[256];
        strcpy_s(logFileBuffer, m_TempSettings.logFile.c_str());
        if (TimeGUI::InputText("Log File", logFileBuffer, sizeof(logFileBuffer))) {
            m_TempSettings.logFile = logFileBuffer;
            changed = true;
        }
        
        if (changed) {
            m_SettingsChanged = true;
        }
        
        TimeGUI::Separator();
        TimeGUI::Text("Current Settings:");
        TimeGUI::Text("  File Logging: %s", m_Settings.IsLogToFileEnabled() ? "Enabled" : "Disabled");
        TimeGUI::Text("  Console Logging: %s", m_Settings.IsLogToConsoleEnabled() ? "Enabled" : "Disabled");
        TimeGUI::Text("  Log Level: %s", m_Settings.GetLogLevel().c_str());
        TimeGUI::Text("  Timestamps: %s", m_Settings.IsLogTimestampEnabled() ? "Enabled" : "Disabled");
        TimeGUI::Text("  Log File: %s", m_Settings.GetLogFile().c_str());
    }

    void EngineSettingsLayer::RenderPerformanceTab() {
        TimeGUI::TextColored(m_HeaderColor, "Performance Settings");
        TimeGUI::Separator();
        
        bool changed = false;
        
        // Performance Limits
        changed |= TimeGUI::SliderInt("Max Draw Calls", (int*)&m_TempSettings.maxDrawCalls, 100, 100000, "%d");
        changed |= TimeGUI::SliderInt("Max Triangles", (int*)&m_TempSettings.maxTriangles, 1000, 10000000, "%d");
        changed |= TimeGUI::SliderInt("Max Vertices", (int*)&m_TempSettings.maxVertices, 1000, 20000000, "%d");
        changed |= TimeGUI::SliderInt("Max Textures", (int*)&m_TempSettings.maxTextures, 10, 10000, "%d");
        changed |= TimeGUI::SliderInt("Max Shaders", (int*)&m_TempSettings.maxShaders, 1, 1000, "%d");
        
        if (changed) {
            m_SettingsChanged = true;
        }
        
        TimeGUI::Separator();
        TimeGUI::Text("Current Settings:");
        TimeGUI::Text("  Max Draw Calls: %u", m_Settings.GetMaxDrawCalls());
        TimeGUI::Text("  Max Triangles: %u", m_Settings.GetMaxTriangles());
        TimeGUI::Text("  Max Vertices: %u", m_Settings.GetMaxVertices());
        TimeGUI::Text("  Max Textures: %u", m_Settings.GetMaxTextures());
        TimeGUI::Text("  Max Shaders: %u", m_Settings.GetMaxShaders());
    }

    void EngineSettingsLayer::RenderDebugTab() {
        TimeGUI::TextColored(m_HeaderColor, "Debug Settings");
        TimeGUI::Separator();
        
        bool changed = false;
        
        // Debug Options
        changed |= TimeGUI::Checkbox("Debug Mode", &m_TempSettings.debugMode);
        changed |= TimeGUI::Checkbox("Show FPS", &m_TempSettings.showFPS);
        changed |= TimeGUI::Checkbox("Show Performance Metrics", &m_TempSettings.showPerformanceMetrics);
        changed |= TimeGUI::Checkbox("Show Debug Info", &m_TempSettings.showDebugInfo);
        changed |= TimeGUI::Checkbox("Show Wireframe", &m_TempSettings.showWireframe);
        changed |= TimeGUI::Checkbox("Show Bounding Boxes", &m_TempSettings.showBoundingBoxes);
        
        if (changed) {
            m_SettingsChanged = true;
        }
        
        TimeGUI::Separator();
        TimeGUI::Text("Current Settings:");
        TimeGUI::Text("  Debug Mode: %s", m_Settings.IsDebugModeEnabled() ? "Enabled" : "Disabled");
        TimeGUI::Text("  Show FPS: %s", m_Settings.IsShowFPSEnabled() ? "Enabled" : "Disabled");
        TimeGUI::Text("  Show Performance Metrics: %s", m_Settings.IsShowPerformanceMetricsEnabled() ? "Enabled" : "Disabled");
        TimeGUI::Text("  Show Debug Info: %s", m_Settings.IsShowDebugInfoEnabled() ? "Enabled" : "Disabled");
        TimeGUI::Text("  Show Wireframe: %s", m_Settings.IsShowWireframeEnabled() ? "Enabled" : "Disabled");
        TimeGUI::Text("  Show Bounding Boxes: %s", m_Settings.IsShowBoundingBoxesEnabled() ? "Enabled" : "Disabled");
    }

    void EngineSettingsLayer::RenderWindowTab() {
        TimeGUI::TextColored(m_HeaderColor, "Window Settings");
        TimeGUI::Separator();
        
        bool changed = false;
        
        // Window Properties
        char titleBuffer[256];
        strcpy_s(titleBuffer, m_TempSettings.windowTitle.c_str());
        if (TimeGUI::InputText("Window Title", titleBuffer, sizeof(titleBuffer))) {
            m_TempSettings.windowTitle = titleBuffer;
            changed = true;
        }
        
        changed |= TimeGUI::SliderInt("Window Width", (int*)&m_TempSettings.windowWidth, 800, 2560, "%d");
        changed |= TimeGUI::SliderInt("Window Height", (int*)&m_TempSettings.windowHeight, 600, 1440, "%d");
        changed |= TimeGUI::Checkbox("Resizable", &m_TempSettings.windowResizable);
        changed |= TimeGUI::Checkbox("Fullscreen", &m_TempSettings.windowFullscreen);
        changed |= TimeGUI::Checkbox("Maximized", &m_TempSettings.windowMaximized);
        
        if (changed) {
            m_SettingsChanged = true;
        }
        
        TimeGUI::Separator();
        TimeGUI::Text("Current Settings:");
        TimeGUI::Text("  Title: %s", m_Settings.GetWindowTitle().c_str());
        TimeGUI::Text("  Size: %ux%u", m_Settings.GetWindowWidth(), m_Settings.GetWindowHeight());
        TimeGUI::Text("  Resizable: %s", m_Settings.IsWindowResizable() ? "Yes" : "No");
        TimeGUI::Text("  Fullscreen: %s", m_Settings.IsWindowFullscreen() ? "Yes" : "No");
        TimeGUI::Text("  Maximized: %s", m_Settings.IsWindowMaximized() ? "Yes" : "No");
    }

    void EngineSettingsLayer::RenderRenderingTab() {
        TimeGUI::TextColored(m_HeaderColor, "Rendering Settings");
        TimeGUI::Separator();
        
        bool changed = false;
        
        // Clear Color
        changed |= TimeGUI::ColorEdit4("Clear Color", m_TempSettings.clearColor);
        
        // Rendering Options
        changed |= TimeGUI::Checkbox("Depth Test", &m_TempSettings.depthTest);
        changed |= TimeGUI::Checkbox("Blending", &m_TempSettings.blending);
        changed |= TimeGUI::Checkbox("Culling", &m_TempSettings.culling);
        changed |= TimeGUI::Checkbox("Multisampling", &m_TempSettings.multisampling);
        
        if (changed) {
            m_SettingsChanged = true;
        }
        
        TimeGUI::Separator();
        TimeGUI::Text("Current Settings:");
        const float* clearColor = m_Settings.GetClearColor();
        TimeGUI::Text("  Clear Color: (%.2f, %.2f, %.2f, %.2f)", 
                   clearColor[0], clearColor[1], clearColor[2], clearColor[3]);
        TimeGUI::Text("  Depth Test: %s", m_Settings.IsDepthTestEnabled() ? "Enabled" : "Disabled");
        TimeGUI::Text("  Blending: %s", m_Settings.IsBlendingEnabled() ? "Enabled" : "Disabled");
        TimeGUI::Text("  Culling: %s", m_Settings.IsCullingEnabled() ? "Enabled" : "Disabled");
        TimeGUI::Text("  Multisampling: %s", m_Settings.IsMultisamplingEnabled() ? "Enabled" : "Disabled");
    }

    void EngineSettingsLayer::RenderInputTab() {
        TimeGUI::TextColored(m_HeaderColor, "Input Settings");
        TimeGUI::Separator();
        
        bool changed = false;
        
        // Input Options
        changed |= TimeGUI::SliderFloat("Mouse Sensitivity", &m_TempSettings.mouseSensitivity, 0.1f, 5.0f, "%.2f");
        changed |= TimeGUI::Checkbox("Keyboard Repeat", &m_TempSettings.keyboardRepeat);
        changed |= TimeGUI::Checkbox("Mouse Acceleration", &m_TempSettings.mouseAcceleration);
        
        if (changed) {
            m_SettingsChanged = true;
        }
        
        TimeGUI::Separator();
        TimeGUI::Text("Current Settings:");
        TimeGUI::Text("  Mouse Sensitivity: %.2f", m_Settings.GetMouseSensitivity());
        TimeGUI::Text("  Keyboard Repeat: %s", m_Settings.IsKeyboardRepeatEnabled() ? "Enabled" : "Disabled");
        TimeGUI::Text("  Mouse Acceleration: %s", m_Settings.IsMouseAccelerationEnabled() ? "Enabled" : "Disabled");
    }

    void EngineSettingsLayer::RenderAudioTab() {
        TimeGUI::TextColored(m_HeaderColor, "Audio Settings");
        TimeGUI::Separator();
        
        bool changed = false;
        
        // Audio Options
        changed |= TimeGUI::Checkbox("Audio Enabled", &m_TempSettings.audioEnabled);
        changed |= TimeGUI::SliderFloat("Audio Volume", &m_TempSettings.audioVolume, 0.0f, 1.0f, "%.2f");
        changed |= TimeGUI::SliderInt("Sample Rate", (int*)&m_TempSettings.audioSampleRate, 8000, 96000, "%d Hz");
        changed |= TimeGUI::SliderInt("Channels", (int*)&m_TempSettings.audioChannels, 1, 8, "%d");
        
        if (changed) {
            m_SettingsChanged = true;
        }
        
        TimeGUI::Separator();
        TimeGUI::Text("Current Settings:");
        TimeGUI::Text("  Audio Enabled: %s", m_Settings.IsAudioEnabled() ? "Yes" : "No");
        TimeGUI::Text("  Volume: %.2f", m_Settings.GetAudioVolume());
        TimeGUI::Text("  Sample Rate: %u Hz", m_Settings.GetAudioSampleRate());
        TimeGUI::Text("  Channels: %u", m_Settings.GetAudioChannels());
    }

    void EngineSettingsLayer::RenderFileIOTab() {
        TimeGUI::TextColored(m_HeaderColor, "File I/O Settings");
        TimeGUI::Separator();
        
        bool changed = false;
        
        // File Paths
        char assetPathBuffer[256];
        strcpy_s(assetPathBuffer, m_TempSettings.assetPath.c_str());
        if (TimeGUI::InputText("Asset Path", assetPathBuffer, sizeof(assetPathBuffer))) {
            m_TempSettings.assetPath = assetPathBuffer;
            changed = true;
        }
        
        char configPathBuffer[256];
        strcpy_s(configPathBuffer, m_TempSettings.configPath.c_str());
        if (TimeGUI::InputText("Config Path", configPathBuffer, sizeof(configPathBuffer))) {
            m_TempSettings.configPath = configPathBuffer;
            changed = true;
        }
        
        char logPathBuffer[256];
        strcpy_s(logPathBuffer, m_TempSettings.logPath.c_str());
        if (TimeGUI::InputText("Log Path", logPathBuffer, sizeof(logPathBuffer))) {
            m_TempSettings.logPath = logPathBuffer;
            changed = true;
        }
        
        char savePathBuffer[256];
        strcpy_s(savePathBuffer, m_TempSettings.savePath.c_str());
        if (TimeGUI::InputText("Save Path", savePathBuffer, sizeof(savePathBuffer))) {
            m_TempSettings.savePath = savePathBuffer;
            changed = true;
        }
        
        if (changed) {
            m_SettingsChanged = true;
        }
        
        TimeGUI::Separator();
        TimeGUI::Text("Current Settings:");
        TimeGUI::Text("  Asset Path: %s", m_Settings.GetAssetPath().c_str());
        TimeGUI::Text("  Config Path: %s", m_Settings.GetConfigPath().c_str());
        TimeGUI::Text("  Log Path: %s", m_Settings.GetLogPath().c_str());
        TimeGUI::Text("  Save Path: %s", m_Settings.GetSavePath().c_str());
    }

    void EngineSettingsLayer::RenderSettingsTab() {
        TimeGUI::TextColored(m_HeaderColor, "Settings Management");
        TimeGUI::Separator();
        
        // Action Buttons
        if (TimeGUI::Button("Apply Settings", TEVector2(120, 30))) {
            ApplyCurrentSettings();
        }
        
        TimeGUI::SameLine();
        if (TimeGUI::Button("Reset to Defaults", TEVector2(120, 30))) {
            m_ShowConfirmationDialog = true;
        }
        
        TimeGUI::SameLine();
        if (TimeGUI::Button("Validate Settings", TEVector2(120, 30))) {
            ValidateCurrentSettings();
        }
        
        TimeGUI::Separator();
        
        // File Operations
        char settingsFileBuffer[256];
        strcpy_s(settingsFileBuffer, m_SelectedSettingsFile.c_str());
        if (TimeGUI::InputText("Settings File", settingsFileBuffer, sizeof(settingsFileBuffer))) {
            m_SelectedSettingsFile = settingsFileBuffer;
        }
        
        if (TimeGUI::Button("Load Settings", TEVector2(120, 30))) {
            LoadSettingsFromFile();
        }
        
        TimeGUI::SameLine();
        if (TimeGUI::Button("Save Settings", TEVector2(120, 30))) {
            SaveSettingsToFile();
        }
        
        TimeGUI::Separator();
        
        // Status Information
        if (m_SettingsChanged) {
            TimeGUI::TextColored(m_WarningColor, "Settings have been modified but not applied.");
        } else {
            TimeGUI::TextColored(m_SuccessColor, "Settings are up to date.");
        }
        
        TimeGUI::Text("Validation Status:");
        if (m_Settings.ValidateSettings()) {
            TimeGUI::TextColored(m_SuccessColor, "  ✓ All settings are valid");
        } else {
            TimeGUI::TextColored(m_ErrorColor, "  ✗ Settings have validation errors");
            TimeGUI::TextWrapped("  %s", m_Settings.GetValidationErrors().c_str());
        }
    }

    void EngineSettingsLayer::ApplyCurrentSettings() {
        // Apply frame rate settings
        m_Settings.SetTargetFrameRate(m_TempSettings.targetFrameRate);
        m_Settings.SetUnlimitedFrameRate(m_TempSettings.unlimitedFrameRate);
        m_Settings.SetVSync(m_TempSettings.vSyncEnabled);
        m_Settings.SetFrameRateLimit(m_TempSettings.frameRateLimit);
        
        // Apply logging settings
        m_Settings.SetLogToFile(m_TempSettings.logToFile);
        m_Settings.SetLogToConsole(m_TempSettings.logToConsole);
        m_Settings.SetLogLevel(m_TempSettings.logLevel);
        m_Settings.SetLogTimestamp(m_TempSettings.logTimestamp);
        m_Settings.SetLogFile(m_TempSettings.logFile);
        
        // Apply performance settings
        m_Settings.SetMaxDrawCalls(m_TempSettings.maxDrawCalls);
        m_Settings.SetMaxTriangles(m_TempSettings.maxTriangles);
        m_Settings.SetMaxVertices(m_TempSettings.maxVertices);
        m_Settings.SetMaxTextures(m_TempSettings.maxTextures);
        m_Settings.SetMaxShaders(m_TempSettings.maxShaders);
        
        // Apply debug settings
        m_Settings.SetDebugMode(m_TempSettings.debugMode);
        m_Settings.SetShowFPS(m_TempSettings.showFPS);
        m_Settings.SetShowPerformanceMetrics(m_TempSettings.showPerformanceMetrics);
        m_Settings.SetShowDebugInfo(m_TempSettings.showDebugInfo);
        m_Settings.SetShowWireframe(m_TempSettings.showWireframe);
        m_Settings.SetShowBoundingBoxes(m_TempSettings.showBoundingBoxes);
        
        // Apply window settings
        m_Settings.SetWindowTitle(m_TempSettings.windowTitle);
        m_Settings.SetWindowSize(m_TempSettings.windowWidth, m_TempSettings.windowHeight);
        m_Settings.SetWindowResizable(m_TempSettings.windowResizable);
        m_Settings.SetWindowFullscreen(m_TempSettings.windowFullscreen);
        m_Settings.SetWindowMaximized(m_TempSettings.windowMaximized);
        
        // Apply rendering settings
        m_Settings.SetClearColor(m_TempSettings.clearColor[0], m_TempSettings.clearColor[1], 
                                m_TempSettings.clearColor[2], m_TempSettings.clearColor[3]);
        m_Settings.SetDepthTest(m_TempSettings.depthTest);
        m_Settings.SetBlending(m_TempSettings.blending);
        m_Settings.SetCulling(m_TempSettings.culling);
        m_Settings.SetMultisampling(m_TempSettings.multisampling);
        
        // Apply input settings
        m_Settings.SetMouseSensitivity(m_TempSettings.mouseSensitivity);
        m_Settings.SetKeyboardRepeat(m_TempSettings.keyboardRepeat);
        m_Settings.SetMouseAcceleration(m_TempSettings.mouseAcceleration);
        
        // Apply audio settings
        m_Settings.SetAudioEnabled(m_TempSettings.audioEnabled);
        m_Settings.SetAudioVolume(m_TempSettings.audioVolume);
        m_Settings.SetAudioSampleRate(m_TempSettings.audioSampleRate);
        m_Settings.SetAudioChannels(m_TempSettings.audioChannels);
        
        // Apply file I/O settings
        m_Settings.SetAssetPath(m_TempSettings.assetPath);
        m_Settings.SetConfigPath(m_TempSettings.configPath);
        m_Settings.SetLogPath(m_TempSettings.logPath);
        m_Settings.SetSavePath(m_TempSettings.savePath);
        
        // Apply all settings
        m_Settings.ApplySettings();
        
        m_SettingsChanged = false;
        TE_CORE_INFO("All engine settings applied successfully");
    }

    void EngineSettingsLayer::ResetToDefaults() {
        m_Settings.ResetToDefaults();
        
        // Update temporary settings to match
        OnAttach();
        
        m_SettingsChanged = false;
        TE_CORE_INFO("Engine settings reset to defaults");
    }

    void EngineSettingsLayer::LoadSettingsFromFile() {
        m_Settings.LoadFromFile(m_SelectedSettingsFile);
        
        // Update temporary settings to match
        OnAttach();
        
        m_SettingsChanged = false;
        TE_CORE_INFO("Settings loaded from file: {0}", m_SelectedSettingsFile);
    }

    void EngineSettingsLayer::SaveSettingsToFile() {
        m_Settings.SaveToFile(m_SelectedSettingsFile);
        TE_CORE_INFO("Settings saved to file: {0}", m_SelectedSettingsFile);
    }

    void EngineSettingsLayer::ValidateCurrentSettings() {
        if (!m_Settings.ValidateSettings()) {
            m_ValidationErrorText = m_Settings.GetValidationErrors();
            m_ShowValidationErrors = true;
            TE_CORE_WARN("Settings validation failed: {0}", m_ValidationErrorText);
        } else {
            TE_CORE_INFO("Settings validation passed");
        }
    }

} // namespace TE 