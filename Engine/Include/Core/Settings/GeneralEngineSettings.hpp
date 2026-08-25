#pragma once

#include "Core/Settings/EngineSettings.hpp"
#include "Core/Settings/EngineSettingsRegistry.hpp"


/**
 * GeneralEngineSettings - Concrete general engine settings subclass
 */
class TE_API GeneralEngineSettings : public EngineSettings
{
public:
    GeneralEngineSettings();
    virtual ~GeneralEngineSettings() = default;

    virtual TEString GetCategoryName() const override { return "Engine"; }
    virtual TEString GetSectionName() const override { return "General"; }
    virtual TEString GetDisplayName() const override { return "General Engine Settings"; }
    virtual TEString GetDescription() const override { return "Core engine frame rate, logging, performance, and windowing configurations."; }

    static GeneralEngineSettings &Get();

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
    void SetLogLevel(const TEString &level);
    void SetLogCategory(const TEString &category, bool enabled);
    void SetLogTimestamp(bool enabled);
    void SetLogFile(const TEString &filename);

    bool IsLogToFileEnabled() const { return m_LogToFile; }
    bool IsLogToConsoleEnabled() const { return m_LogToConsole; }
    const TEString &GetLogLevel() const { return m_LogLevel; }
    bool IsLogCategoryEnabled(const TEString &category) const;
    const TEMap<TEString, bool> &GetLogCategories() const { return m_LogCategories; }
    TEMap<TEString, bool> &GetLogCategories() { return m_LogCategories; }
    bool IsLogTimestampEnabled() const { return m_LogTimestamp; }
    const TEString &GetLogFile() const { return m_LogFile; }

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
    void SetWindowTitle(const TEString &title);
    void SetWindowSize(uint32_t width, uint32_t height);
    void SetWindowResizable(bool resizable);
    void SetWindowFullscreen(bool fullscreen);
    void SetWindowMaximized(bool maximized);

    const TEString &GetWindowTitle() const { return m_WindowTitle; }
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

    const float *GetClearColor() const { return m_ClearColor; }
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
    void SetAssetPath(const TEString &path);
    void SetConfigPath(const TEString &path);
    void SetLogPath(const TEString &path);
    void SetSavePath(const TEString &path);

    const TEString &GetAssetPath() const { return m_AssetPath; }
    const TEString &GetConfigPath() const { return m_ConfigPath; }
    const TEString &GetLogPath() const { return m_LogPath; }
    const TEString &GetSavePath() const { return m_SavePath; }

    // ===== Utility Methods =====
    void LoadFromFile(const TEString &filename);
    void SaveToFile(const TEString &filename);
    virtual void ResetToDefaults() override;
    void ApplySettings();

    // ===== Validation =====
    virtual bool ValidateSettings() const override;
    virtual TEString GetValidationErrors() const override;

private:
    // ===== Frame Rate Settings =====
    float m_TargetFrameRate = 60.0f;
    bool m_UnlimitedFrameRate = false;
    bool m_VSyncEnabled = true;
    float m_FrameRateLimit = 60.0f;

    // ===== Logging Settings =====
    bool m_LogToFile = true;
    bool m_LogToConsole = true;
    TEString m_LogLevel = "INFO";
    TEMap<TEString, bool> m_LogCategories;
    bool m_LogTimestamp = true;
    TEString m_LogFile = "TimeEngineLog.json";

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
    TEString m_WindowTitle = "TimeEngine";
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
    TEString m_AssetPath = "assets/";
    TEString m_ConfigPath = "config/";
    TEString m_LogPath = "logs/";
    TEString m_SavePath = "saves/";

    // ===== Internal Methods =====
    void InitializeDefaultLogCategories();
    void ValidateFrameRateSettings();
    void ValidateLogSettings();
    void ValidatePerformanceSettings();
};

