#pragma once

#include "Core/PreRequisites.h"
#include <string>
#include <unordered_map>

namespace TE {

    // ===== FPS Enums =====
    enum class TargetFPS : uint32_t {
        FPS_30 = 30,
        FPS_60 = 60,
        FPS_75 = 75,
        FPS_90 = 90,
        FPS_120 = 120,
        FPS_144 = 144,
        FPS_165 = 165,
        FPS_240 = 240,
        FPS_UNLIMITED = 0
    };

    enum class FrameRateLimit : uint32_t {
        LIMIT_30 = 30,
        LIMIT_60 = 60,
        LIMIT_75 = 75,
        LIMIT_90 = 90,
        LIMIT_120 = 120,
        LIMIT_144 = 144,
        LIMIT_165 = 165,
        LIMIT_240 = 240,
        LIMIT_UNLIMITED = 0
    };

    // ===== Log Level Enum =====
    enum class LogLevel : uint32_t {
        DEBUG = 0,
        INFO = 1,
        WARNING = 2,
        ERROR = 3,
        CRITICAL = 4
    };

    // ===== Audio Sample Rate Enum =====
    enum class AudioSampleRate : uint32_t {
        SR_8000 = 8000,
        SR_11025 = 11025,
        SR_16000 = 16000,
        SR_22050 = 22050,
        SR_44100 = 44100,
        SR_48000 = 48000,
        SR_96000 = 96000
    };

    // ===== Audio Channels Enum =====
    enum class AudioChannels : uint32_t {
        MONO = 1,
        STEREO = 2,
        SURROUND_5_1 = 6,
        SURROUND_7_1 = 8
    };

    /**
     * EngineSettings - Global engine configuration class
     * Controls frame rate, logging options, and other global engine settings
     */
    class TE_API EngineSettings {
    public:
        // ===== Singleton Pattern =====
        static EngineSettings& Get();
        
        // ===== Frame Rate Settings =====
        void SetTargetFrameRate(float fps);
        void SetUnlimitedFrameRate(bool unlimited);
        void SetVSync(bool enabled);
        void SetFrameRateLimit(float fps);
        
        float GetTargetFrameRate() const { return m_TargetFrameRate; }
        bool IsUnlimitedFrameRate() const { return m_UnlimitedFrameRate; }
        bool IsVSyncEnabled() const { return m_VSyncEnabled; }
        float GetFrameRateLimit() const { return m_FrameRateLimit; }
        
        // ===== Logging Settings =====
        void SetLogToFile(bool enabled);
        void SetLogToConsole(bool enabled);
        void SetLogLevel(const std::string& level);
        void SetLogCategory(const std::string& category, bool enabled);
        void SetLogTimestamp(bool enabled);
        void SetLogFile(const std::string& filename);
        
        bool IsLogToFileEnabled() const { return m_LogToFile; }
        bool IsLogToConsoleEnabled() const { return m_LogToConsole; }
        const std::string& GetLogLevel() const { return m_LogLevel; }
        bool IsLogCategoryEnabled(const std::string& category) const;
        bool IsLogTimestampEnabled() const { return m_LogTimestamp; }
        const std::string& GetLogFile() const { return m_LogFile; }
        
        // ===== Performance Settings =====
        void SetMaxDrawCalls(uint32_t max);
        void SetMaxTriangles(uint32_t max);
        void SetMaxVertices(uint32_t max);
        void SetMaxTextures(uint32_t max);
        void SetMaxShaders(uint32_t max);
        
        uint32_t GetMaxDrawCalls() const { return m_MaxDrawCalls; }
        uint32_t GetMaxTriangles() const { return m_MaxTriangles; }
        uint32_t GetMaxVertices() const { return m_MaxVertices; }
        uint32_t GetMaxTextures() const { return m_MaxTextures; }
        uint32_t GetMaxShaders() const { return m_MaxShaders; }
        
        // ===== Debug Settings =====
        void SetDebugMode(bool enabled);
        void SetShowFPS(bool enabled);
        void SetShowPerformanceMetrics(bool enabled);
        void SetShowDebugInfo(bool enabled);
        void SetShowWireframe(bool enabled);
        void SetShowBoundingBoxes(bool enabled);
        
        bool IsDebugModeEnabled() const { return m_DebugMode; }
        bool IsShowFPSEnabled() const { return m_ShowFPS; }
        bool IsShowPerformanceMetricsEnabled() const { return m_ShowPerformanceMetrics; }
        bool IsShowDebugInfoEnabled() const { return m_ShowDebugInfo; }
        bool IsShowWireframeEnabled() const { return m_ShowWireframe; }
        bool IsShowBoundingBoxesEnabled() const { return m_ShowBoundingBoxes; }
        
        // ===== Window Settings =====
        void SetWindowTitle(const std::string& title);
        void SetWindowSize(uint32_t width, uint32_t height);
        void SetWindowResizable(bool resizable);
        void SetWindowFullscreen(bool fullscreen);
        void SetWindowMaximized(bool maximized);
        
        const std::string& GetWindowTitle() const { return m_WindowTitle; }
        uint32_t GetWindowWidth() const { return m_WindowWidth; }
        uint32_t GetWindowHeight() const { return m_WindowHeight; }
        bool IsWindowResizable() const { return m_WindowResizable; }
        bool IsWindowFullscreen() const { return m_WindowFullscreen; }
        bool IsWindowMaximized() const { return m_WindowMaximized; }
        
        // ===== Rendering Settings =====
        void SetClearColor(float r, float g, float b, float a);
        void SetDepthTest(bool enabled);
        void SetBlending(bool enabled);
        void SetCulling(bool enabled);
        void SetMultisampling(bool enabled);
        
        const float* GetClearColor() const { return m_ClearColor; }
        bool IsDepthTestEnabled() const { return m_DepthTest; }
        bool IsBlendingEnabled() const { return m_Blending; }
        bool IsCullingEnabled() const { return m_Culling; }
        bool IsMultisamplingEnabled() const { return m_Multisampling; }
        
        // ===== Input Settings =====
        void SetMouseSensitivity(float sensitivity);
        void SetKeyboardRepeat(bool enabled);
        void SetMouseAcceleration(bool enabled);
        
        float GetMouseSensitivity() const { return m_MouseSensitivity; }
        bool IsKeyboardRepeatEnabled() const { return m_KeyboardRepeat; }
        bool IsMouseAccelerationEnabled() const { return m_MouseAcceleration; }
        
        // ===== Audio Settings =====
        void SetAudioEnabled(bool enabled);
        void SetAudioVolume(float volume);
        void SetAudioSampleRate(uint32_t sampleRate);
        void SetAudioChannels(uint32_t channels);
        
        bool IsAudioEnabled() const { return m_AudioEnabled; }
        float GetAudioVolume() const { return m_AudioVolume; }
        uint32_t GetAudioSampleRate() const { return m_AudioSampleRate; }
        uint32_t GetAudioChannels() const { return m_AudioChannels; }
        
        // ===== File I/O Settings =====
        void SetAssetPath(const std::string& path);
        void SetConfigPath(const std::string& path);
        void SetLogPath(const std::string& path);
        void SetSavePath(const std::string& path);
        
        const std::string& GetAssetPath() const { return m_AssetPath; }
        const std::string& GetConfigPath() const { return m_ConfigPath; }
        const std::string& GetLogPath() const { return m_LogPath; }
        const std::string& GetSavePath() const { return m_SavePath; }
        
        // ===== Utility Methods =====
        void LoadFromFile(const std::string& filename);
        void SaveToFile(const std::string& filename);
        void ResetToDefaults();
        void ApplySettings();
        
        // ===== Validation =====
        bool ValidateSettings() const;
        std::string GetValidationErrors() const;

    private:
        EngineSettings();
        ~EngineSettings() = default;
        EngineSettings(const EngineSettings&) = delete;
        EngineSettings& operator=(const EngineSettings&) = delete;

        // ===== Frame Rate Settings =====
        float m_TargetFrameRate = 60.0f;
        bool m_UnlimitedFrameRate = false;
        bool m_VSyncEnabled = true;
        float m_FrameRateLimit = 60.0f;
        
        // ===== Logging Settings =====
        bool m_LogToFile = true;
        bool m_LogToConsole = true;
        std::string m_LogLevel = "INFO";
        std::unordered_map<std::string, bool> m_LogCategories;
        bool m_LogTimestamp = true;
        std::string m_LogFile = "TimeEngineLog.json";
        
        // ===== Performance Settings =====
        uint32_t m_MaxDrawCalls = 10000;
        uint32_t m_MaxTriangles = 1000000;
        uint32_t m_MaxVertices = 2000000;
        uint32_t m_MaxTextures = 1000;
        uint32_t m_MaxShaders = 100;
        
        // ===== Debug Settings =====
        bool m_DebugMode = false;
        bool m_ShowFPS = true;
        bool m_ShowPerformanceMetrics = true;
        bool m_ShowDebugInfo = false;
        bool m_ShowWireframe = false;
        bool m_ShowBoundingBoxes = false;
        
        // ===== Window Settings =====
        std::string m_WindowTitle = "TimeEngine";
        uint32_t m_WindowWidth = 1280;
        uint32_t m_WindowHeight = 720;
        bool m_WindowResizable = true;
        bool m_WindowFullscreen = false;
        bool m_WindowMaximized = false;
        
        // ===== Rendering Settings =====
        float m_ClearColor[4] = {0.0f, 0.0f, 0.0f, 1.0f};
        bool m_DepthTest = true;
        bool m_Blending = true;
        bool m_Culling = true;
        bool m_Multisampling = false;
        
        // ===== Input Settings =====
        float m_MouseSensitivity = 1.0f;
        bool m_KeyboardRepeat = true;
        bool m_MouseAcceleration = false;
        
        // ===== Audio Settings =====
        bool m_AudioEnabled = true;
        float m_AudioVolume = 1.0f;
        uint32_t m_AudioSampleRate = 44100;
        uint32_t m_AudioChannels = 2;
        
        // ===== File I/O Settings =====
        std::string m_AssetPath = "assets/";
        std::string m_ConfigPath = "config/";
        std::string m_LogPath = "logs/";
        std::string m_SavePath = "saves/";
        
        // ===== Internal Methods =====
        void InitializeDefaultLogCategories();
        void ValidateFrameRateSettings();
        void ValidateLogSettings();
        void ValidatePerformanceSettings();
    };

} // namespace TE 