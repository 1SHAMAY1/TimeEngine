#include "Core/EngineSettings.hpp"
#include "Core/Log.h"
#include "Core/Application.h"
#include "Window/IWindow.hpp"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <filesystem>

namespace TE {

    // ===== Singleton Instance =====
    static EngineSettings* s_Instance = nullptr;

    EngineSettings::EngineSettings() {
        InitializeDefaultLogCategories();
        TE_CORE_INFO("EngineSettings initialized with default values");
    }

    EngineSettings& EngineSettings::Get() {
        if (!s_Instance) {
            s_Instance = new EngineSettings();
        }
        return *s_Instance;
    }

    // ===== Frame Rate Settings =====
    void EngineSettings::SetTargetFrameRate(float fps) {
        if (fps > 0.0f) {
            m_TargetFrameRate = fps;
            ValidateFrameRateSettings();
            TE_CORE_INFO("Target frame rate set to: {0} FPS", fps);
        } else {
            TE_CORE_WARN("Invalid frame rate: {0}. Must be greater than 0.", fps);
        }
    }

    void EngineSettings::SetUnlimitedFrameRate(bool unlimited) {
        m_UnlimitedFrameRate = unlimited;
        if (unlimited) {
            TE_CORE_INFO("Unlimited frame rate enabled");
        } else {
            TE_CORE_INFO("Frame rate limited to: {0} FPS", m_TargetFrameRate);
        }
    }

    void EngineSettings::SetVSync(bool enabled) {
        m_VSyncEnabled = enabled;
        if (Application::Get().GetWindow().IsVSync() != enabled) {
            Application::Get().GetWindow().SetVSync(enabled);
            TE_CORE_INFO("VSync {0}", enabled ? "enabled" : "disabled");
        }
    }

    void EngineSettings::SetFrameRateLimit(float fps) {
        if (fps > 0.0f) {
            m_FrameRateLimit = fps;
            ValidateFrameRateSettings();
            TE_CORE_INFO("Frame rate limit set to: {0} FPS", fps);
        } else {
            TE_CORE_WARN("Invalid frame rate limit: {0}. Must be greater than 0.", fps);
        }
    }

    // ===== Logging Settings =====
    void EngineSettings::SetLogToFile(bool enabled) {
        m_LogToFile = enabled;
        TE_CORE_INFO("Log to file {0}", enabled ? "enabled" : "disabled");
    }

    void EngineSettings::SetLogToConsole(bool enabled) {
        m_LogToConsole = enabled;
        TE_CORE_INFO("Log to console {0}", enabled ? "enabled" : "disabled");
    }

    void EngineSettings::SetLogLevel(const std::string& level) {
        std::vector<std::string> validLevels = {"DEBUG", "INFO", "WARNING", "ERROR", "CRITICAL"};
        if (std::find(validLevels.begin(), validLevels.end(), level) != validLevels.end()) {
            m_LogLevel = level;
            TE_CORE_INFO("Log level set to: {0}", level);
        } else {
            TE_CORE_WARN("Invalid log level: {0}. Valid levels are: DEBUG, INFO, WARNING, ERROR, CRITICAL", level);
        }
    }

    void EngineSettings::SetLogCategory(const std::string& category, bool enabled) {
        m_LogCategories[category] = enabled;
        TE_CORE_INFO("Log category '{0}' {1}", category, enabled ? "enabled" : "disabled");
    }

    bool EngineSettings::IsLogCategoryEnabled(const std::string& category) const {
        auto it = m_LogCategories.find(category);
        return it != m_LogCategories.end() ? it->second : true; // Default to enabled if not found
    }

    void EngineSettings::SetLogTimestamp(bool enabled) {
        m_LogTimestamp = enabled;
        TE_CORE_INFO("Log timestamps {0}", enabled ? "enabled" : "disabled");
    }

    void EngineSettings::SetLogFile(const std::string& filename) {
        m_LogFile = filename;
        TE_CORE_INFO("Log file set to: {0}", filename);
    }

    // ===== Performance Settings =====
    void EngineSettings::SetMaxDrawCalls(uint32_t max) {
        m_MaxDrawCalls = max;
        TE_CORE_INFO("Max draw calls set to: {0}", max);
    }

    void EngineSettings::SetMaxTriangles(uint32_t max) {
        m_MaxTriangles = max;
        TE_CORE_INFO("Max triangles set to: {0}", max);
    }

    void EngineSettings::SetMaxVertices(uint32_t max) {
        m_MaxVertices = max;
        TE_CORE_INFO("Max vertices set to: {0}", max);
    }

    void EngineSettings::SetMaxTextures(uint32_t max) {
        m_MaxTextures = max;
        TE_CORE_INFO("Max textures set to: {0}", max);
    }

    void EngineSettings::SetMaxShaders(uint32_t max) {
        m_MaxShaders = max;
        TE_CORE_INFO("Max shaders set to: {0}", max);
    }

    // ===== Debug Settings =====
    void EngineSettings::SetDebugMode(bool enabled) {
        m_DebugMode = enabled;
        TE_CORE_INFO("Debug mode {0}", enabled ? "enabled" : "disabled");
    }

    void EngineSettings::SetShowFPS(bool enabled) {
        m_ShowFPS = enabled;
        TE_CORE_INFO("Show FPS {0}", enabled ? "enabled" : "disabled");
    }

    void EngineSettings::SetShowPerformanceMetrics(bool enabled) {
        m_ShowPerformanceMetrics = enabled;
        TE_CORE_INFO("Show performance metrics {0}", enabled ? "enabled" : "disabled");
    }

    void EngineSettings::SetShowDebugInfo(bool enabled) {
        m_ShowDebugInfo = enabled;
        TE_CORE_INFO("Show debug info {0}", enabled ? "enabled" : "disabled");
    }

    void EngineSettings::SetShowWireframe(bool enabled) {
        m_ShowWireframe = enabled;
        TE_CORE_INFO("Show wireframe {0}", enabled ? "enabled" : "disabled");
    }

    void EngineSettings::SetShowBoundingBoxes(bool enabled) {
        m_ShowBoundingBoxes = enabled;
        TE_CORE_INFO("Show bounding boxes {0}", enabled ? "enabled" : "disabled");
    }

    // ===== Window Settings =====
    void EngineSettings::SetWindowTitle(const std::string& title) {
        m_WindowTitle = title;
        TE_CORE_INFO("Window title set to: {0}", title);
    }

    void EngineSettings::SetWindowSize(uint32_t width, uint32_t height) {
        if (width > 0 && height > 0) {
            m_WindowWidth = width;
            m_WindowHeight = height;
            TE_CORE_INFO("Window size set to: {0}x{1}", width, height);
        } else {
            TE_CORE_WARN("Invalid window size: {0}x{1}. Width and height must be greater than 0.", width, height);
        }
    }

    void EngineSettings::SetWindowResizable(bool resizable) {
        m_WindowResizable = resizable;
        TE_CORE_INFO("Window resizable {0}", resizable ? "enabled" : "disabled");
    }

    void EngineSettings::SetWindowFullscreen(bool fullscreen) {
        m_WindowFullscreen = fullscreen;
        TE_CORE_INFO("Window fullscreen {0}", fullscreen ? "enabled" : "disabled");
    }

    void EngineSettings::SetWindowMaximized(bool maximized) {
        m_WindowMaximized = maximized;
        TE_CORE_INFO("Window maximized {0}", maximized ? "enabled" : "disabled");
    }

    // ===== Rendering Settings =====
    void EngineSettings::SetClearColor(float r, float g, float b, float a) {
        m_ClearColor[0] = std::clamp(r, 0.0f, 1.0f);
        m_ClearColor[1] = std::clamp(g, 0.0f, 1.0f);
        m_ClearColor[2] = std::clamp(b, 0.0f, 1.0f);
        m_ClearColor[3] = std::clamp(a, 0.0f, 1.0f);
        TE_CORE_INFO("Clear color set to: ({0}, {1}, {2}, {3})", r, g, b, a);
    }

    void EngineSettings::SetDepthTest(bool enabled) {
        m_DepthTest = enabled;
        TE_CORE_INFO("Depth test {0}", enabled ? "enabled" : "disabled");
    }

    void EngineSettings::SetBlending(bool enabled) {
        m_Blending = enabled;
        TE_CORE_INFO("Blending {0}", enabled ? "enabled" : "disabled");
    }

    void EngineSettings::SetCulling(bool enabled) {
        m_Culling = enabled;
        TE_CORE_INFO("Culling {0}", enabled ? "enabled" : "disabled");
    }

    void EngineSettings::SetMultisampling(bool enabled) {
        m_Multisampling = enabled;
        TE_CORE_INFO("Multisampling {0}", enabled ? "enabled" : "disabled");
    }

    // ===== Input Settings =====
    void EngineSettings::SetMouseSensitivity(float sensitivity) {
        if (sensitivity > 0.0f) {
            m_MouseSensitivity = sensitivity;
            TE_CORE_INFO("Mouse sensitivity set to: {0}", sensitivity);
        } else {
            TE_CORE_WARN("Invalid mouse sensitivity: {0}. Must be greater than 0.", sensitivity);
        }
    }

    void EngineSettings::SetKeyboardRepeat(bool enabled) {
        m_KeyboardRepeat = enabled;
        TE_CORE_INFO("Keyboard repeat {0}", enabled ? "enabled" : "disabled");
    }

    void EngineSettings::SetMouseAcceleration(bool enabled) {
        m_MouseAcceleration = enabled;
        TE_CORE_INFO("Mouse acceleration {0}", enabled ? "enabled" : "disabled");
    }

    // ===== Audio Settings =====
    void EngineSettings::SetAudioEnabled(bool enabled) {
        m_AudioEnabled = enabled;
        TE_CORE_INFO("Audio {0}", enabled ? "enabled" : "disabled");
    }

    void EngineSettings::SetAudioVolume(float volume) {
        if (volume >= 0.0f && volume <= 1.0f) {
            m_AudioVolume = volume;
            TE_CORE_INFO("Audio volume set to: {0}", volume);
        } else {
            TE_CORE_WARN("Invalid audio volume: {0}. Must be between 0.0 and 1.0.", volume);
        }
    }

    void EngineSettings::SetAudioSampleRate(uint32_t sampleRate) {
        if (sampleRate > 0) {
            m_AudioSampleRate = sampleRate;
            TE_CORE_INFO("Audio sample rate set to: {0} Hz", sampleRate);
        } else {
            TE_CORE_WARN("Invalid audio sample rate: {0}. Must be greater than 0.", sampleRate);
        }
    }

    void EngineSettings::SetAudioChannels(uint32_t channels) {
        if (channels > 0 && channels <= 8) {
            m_AudioChannels = channels;
            TE_CORE_INFO("Audio channels set to: {0}", channels);
        } else {
            TE_CORE_WARN("Invalid audio channels: {0}. Must be between 1 and 8.", channels);
        }
    }

    // ===== File I/O Settings =====
    void EngineSettings::SetAssetPath(const std::string& path) {
        m_AssetPath = path;
        TE_CORE_INFO("Asset path set to: {0}", path);
    }

    void EngineSettings::SetConfigPath(const std::string& path) {
        m_ConfigPath = path;
        TE_CORE_INFO("Config path set to: {0}", path);
    }

    void EngineSettings::SetLogPath(const std::string& path) {
        m_LogPath = path;
        TE_CORE_INFO("Log path set to: {0}", path);
    }

    void EngineSettings::SetSavePath(const std::string& path) {
        m_SavePath = path;
        TE_CORE_INFO("Save path set to: {0}", path);
    }

    // ===== Utility Methods =====
    void EngineSettings::LoadFromFile(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            TE_CORE_WARN("Could not open settings file: {0}", filename);
            return;
        }

        // TODO: Implement JSON parsing for settings file
        TE_CORE_INFO("Loading settings from: {0}", filename);
        file.close();
    }

    void EngineSettings::SaveToFile(const std::string& filename) {
        std::ofstream file(filename);
        if (!file.is_open()) {
            TE_CORE_ERROR("Could not create settings file: {0}", filename);
            return;
        }

        // TODO: Implement JSON serialization for settings
        TE_CORE_INFO("Saving settings to: {0}", filename);
        file.close();
    }

    void EngineSettings::ResetToDefaults() {
        // Reset all settings to default values
        m_TargetFrameRate = 60.0f;
        m_UnlimitedFrameRate = false;
        m_VSyncEnabled = true;
        m_FrameRateLimit = 60.0f;
        
        m_LogToFile = true;
        m_LogToConsole = true;
        m_LogLevel = "INFO";
        m_LogTimestamp = true;
        m_LogFile = "TimeEngineLog.json";
        
        m_MaxDrawCalls = 10000;
        m_MaxTriangles = 1000000;
        m_MaxVertices = 2000000;
        m_MaxTextures = 1000;
        m_MaxShaders = 100;
        
        m_DebugMode = false;
        m_ShowFPS = true;
        m_ShowPerformanceMetrics = true;
        m_ShowDebugInfo = false;
        m_ShowWireframe = false;
        m_ShowBoundingBoxes = false;
        
        m_WindowTitle = "TimeEngine";
        m_WindowWidth = 1280;
        m_WindowHeight = 720;
        m_WindowResizable = true;
        m_WindowFullscreen = false;
        m_WindowMaximized = false;
        
        m_ClearColor[0] = 0.0f;
        m_ClearColor[1] = 0.0f;
        m_ClearColor[2] = 0.0f;
        m_ClearColor[3] = 1.0f;
        m_DepthTest = true;
        m_Blending = true;
        m_Culling = true;
        m_Multisampling = false;
        
        m_MouseSensitivity = 1.0f;
        m_KeyboardRepeat = true;
        m_MouseAcceleration = false;
        
        m_AudioEnabled = true;
        m_AudioVolume = 1.0f;
        m_AudioSampleRate = 44100;
        m_AudioChannels = 2;
        
        m_AssetPath = "assets/";
        m_ConfigPath = "config/";
        m_LogPath = "logs/";
        m_SavePath = "saves/";
        
        InitializeDefaultLogCategories();
        
        TE_CORE_INFO("Engine settings reset to defaults");
    }

    void EngineSettings::ApplySettings() {
        // Apply window settings
        if (Application::Get().GetWindow().GetWidth() != m_WindowWidth || 
            Application::Get().GetWindow().GetHeight() != m_WindowHeight) {
            TE_CORE_INFO("Window size change requires application restart");
        }
        
        // Apply VSync setting
        SetVSync(m_VSyncEnabled);
        
        // Apply logging settings
        if (m_LogToFile) {
            TE_CORE_INFO("File logging enabled: {0}", m_LogFile);
        }
        
        if (m_LogToConsole) {
            TE_CORE_INFO("Console logging enabled");
        }
        
        TE_CORE_INFO("Engine settings applied successfully");
    }

    // ===== Validation =====
    bool EngineSettings::ValidateSettings() const {
        return GetValidationErrors().empty();
    }

    std::string EngineSettings::GetValidationErrors() const {
        std::stringstream errors;
        
        if (m_TargetFrameRate <= 0.0f) {
            errors << "Target frame rate must be greater than 0. ";
        }
        
        if (m_FrameRateLimit <= 0.0f) {
            errors << "Frame rate limit must be greater than 0. ";
        }
        
        if (m_WindowWidth == 0 || m_WindowHeight == 0) {
            errors << "Window dimensions must be greater than 0. ";
        }
        
        if (m_AudioVolume < 0.0f || m_AudioVolume > 1.0f) {
            errors << "Audio volume must be between 0.0 and 1.0. ";
        }
        
        if (m_AudioSampleRate == 0) {
            errors << "Audio sample rate must be greater than 0. ";
        }
        
        if (m_AudioChannels == 0 || m_AudioChannels > 8) {
            errors << "Audio channels must be between 1 and 8. ";
        }
        
        return errors.str();
    }

    // ===== Private Methods =====
    void EngineSettings::InitializeDefaultLogCategories() {
        m_LogCategories.clear();
        m_LogCategories["Core"] = true;
        m_LogCategories["Client"] = true;
        m_LogCategories["Renderer"] = true;
        m_LogCategories["Input"] = true;
        m_LogCategories["Audio"] = true;
        m_LogCategories["Physics"] = true;
        m_LogCategories["Network"] = true;
        m_LogCategories["Debug"] = true;
        m_LogCategories["Performance"] = true;
        m_LogCategories["FileIO"] = true;
    }

    void EngineSettings::ValidateFrameRateSettings() {
        if (m_TargetFrameRate > m_FrameRateLimit) {
            TE_CORE_WARN("Target frame rate ({0}) is higher than frame rate limit ({1})", 
                        m_TargetFrameRate, m_FrameRateLimit);
        }
    }

    void EngineSettings::ValidateLogSettings() {
        if (!m_LogToFile && !m_LogToConsole) {
            TE_CORE_WARN("Both file and console logging are disabled");
        }
    }

    void EngineSettings::ValidatePerformanceSettings() {
        if (m_MaxDrawCalls == 0) {
            TE_CORE_WARN("Max draw calls is set to 0");
        }
        if (m_MaxTriangles == 0) {
            TE_CORE_WARN("Max triangles is set to 0");
        }
        if (m_MaxVertices == 0) {
            TE_CORE_WARN("Max vertices is set to 0");
        }
    }

} // namespace TE 