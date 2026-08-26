#include "Core/Application.h"
#include "Core/Log.h"
#include "Core/PreRequisites.h"
#include "Core/Settings/GeneralEngineSettings.hpp"
#include "Utils/TEFileSystem.hpp"
#include "Window/IWindow.hpp"
#include <algorithm>
#include <fstream>
#include <sstream>
TE_REGISTER_SETTINGS(GeneralEngineSettings);

GeneralEngineSettings::GeneralEngineSettings()
{
    InitializeDefaultLogCategories();
    TE_CORE_INFO("GeneralEngineSettings initialized with default values");
}

GeneralEngineSettings &GeneralEngineSettings::Get()
{
    return EngineSettingsRegistry::GetMutable<GeneralEngineSettings>();
}

// ===== Frame Rate Settings =====
void GeneralEngineSettings::SetTargetFrameRate(float fps)
{
    if (fps > 0.0f)
    {
        m_TargetFrameRate = fps;
        ValidateFrameRateSettings();
        TE_CORE_INFO("Target frame rate set to: {0} FPS", fps);
    }
    else
    {
        TE_CORE_WARN("Invalid frame rate: {0}. Must be greater than 0.", fps);
    }
}

void GeneralEngineSettings::SetUnlimitedFrameRate(bool unlimited)
{
    m_UnlimitedFrameRate = unlimited;
    if (unlimited)
    {
        TE_CORE_INFO("Unlimited frame rate enabled");
    }
    else
    {
        TE_CORE_INFO("Frame rate limited to: {0} FPS", m_TargetFrameRate);
    }
}

void GeneralEngineSettings::SetVSync(bool enabled)
{
    m_VSyncEnabled = enabled;
    if (Application::Get().GetWindow().IsVSync() != enabled)
    {
        Application::Get().GetWindow().SetVSync(enabled);
        TE_CORE_INFO("VSync {0}", enabled ? "enabled" : "disabled");
    }
}

void GeneralEngineSettings::SetFrameRateLimit(float fps)
{
    if (fps > 0.0f)
    {
        m_FrameRateLimit = fps;
        ValidateFrameRateSettings();
        TE_CORE_INFO("Frame rate limit set to: {0} FPS", fps);
    }
    else
    {
        TE_CORE_WARN("Invalid frame rate limit: {0}. Must be greater than 0.", fps);
    }
}

// ===== Logging Settings =====
void GeneralEngineSettings::SetLogToFile(bool enabled)
{
    m_LogToFile = enabled;
    TE_CORE_INFO("Log to file {0}", enabled ? "enabled" : "disabled");
}

void GeneralEngineSettings::SetLogToConsole(bool enabled)
{
    m_LogToConsole = enabled;
    TE_CORE_INFO("Log to console {0}", enabled ? "enabled" : "disabled");
}

void GeneralEngineSettings::SetLogLevel(const TEString &level)
{
    TEArray<TEString> validLevels = {"DEBUG", "INFO", "WARNING", "ERROR", "CRITICAL"};
    if (validLevels.Contains(level))
    {
        m_LogLevel = level;
        TE_CORE_INFO("Log level set to: {0}", level);
    }
    else
    {
        TE_CORE_WARN("Invalid log level: {0}. Valid levels are: DEBUG, INFO, WARNING, ERROR, CRITICAL", level);
    }
}

void GeneralEngineSettings::SetLogCategory(const TEString &category, bool enabled)
{
    m_LogCategories[category] = enabled;
    TE_CORE_INFO("Log category '{0}' {1}", category, enabled ? "enabled" : "disabled");
}

bool GeneralEngineSettings::IsLogCategoryEnabled(const TEString &category) const
{
    auto *it = m_LogCategories.Find(category);
    return it != nullptr ? *it : true; // Default to enabled if not found
}

void GeneralEngineSettings::SetLogTimestamp(bool enabled)
{
    m_LogTimestamp = enabled;
    TE_CORE_INFO("Log timestamps {0}", enabled ? "enabled" : "disabled");
}

void GeneralEngineSettings::SetLogFile(const TEString &filename)
{
    m_LogFile = filename;
    TE_CORE_INFO("Log file set to: {0}", filename);
}

// ===== Performance Settings =====
void GeneralEngineSettings::SetMaxDrawCalls(uint32_t max)
{
    m_MaxDrawCalls = max;
    TE_CORE_INFO("Max draw calls set to: {0}", max);
}

void GeneralEngineSettings::SetMaxTriangles(uint32_t max)
{
    m_MaxTriangles = max;
    TE_CORE_INFO("Max triangles set to: {0}", max);
}

void GeneralEngineSettings::SetMaxVertices(uint32_t max)
{
    m_MaxVertices = max;
    TE_CORE_INFO("Max vertices set to: {0}", max);
}

void GeneralEngineSettings::SetMaxTextures(uint32_t max)
{
    m_MaxTextures = max;
    TE_CORE_INFO("Max textures set to: {0}", max);
}

void GeneralEngineSettings::SetMaxShaders(uint32_t max)
{
    m_MaxShaders = max;
    TE_CORE_INFO("Max shaders set to: {0}", max);
}

// ===== Debug Settings =====
void GeneralEngineSettings::SetDebugMode(bool enabled)
{
    m_DebugMode = enabled;
    TE_CORE_INFO("Debug mode {0}", enabled ? "enabled" : "disabled");
}

void GeneralEngineSettings::SetShowFPS(bool enabled)
{
    m_ShowFPS = enabled;
    TE_CORE_INFO("Show FPS {0}", enabled ? "enabled" : "disabled");
}

void GeneralEngineSettings::SetShowPerformanceMetrics(bool enabled)
{
    m_ShowPerformanceMetrics = enabled;
    TE_CORE_INFO("Show performance metrics {0}", enabled ? "enabled" : "disabled");
}

void GeneralEngineSettings::SetShowDebugInfo(bool enabled)
{
    m_ShowDebugInfo = enabled;
    TE_CORE_INFO("Show debug info {0}", enabled ? "enabled" : "disabled");
}

void GeneralEngineSettings::SetShowWireframe(bool enabled)
{
    m_ShowWireframe = enabled;
    TE_CORE_INFO("Show wireframe {0}", enabled ? "enabled" : "disabled");
}

void GeneralEngineSettings::SetShowBoundingBoxes(bool enabled)
{
    m_ShowBoundingBoxes = enabled;
    TE_CORE_INFO("Show bounding boxes {0}", enabled ? "enabled" : "disabled");
}

// ===== Window Settings =====
void GeneralEngineSettings::SetWindowTitle(const TEString &title)
{
    m_WindowTitle = title;
    TE_CORE_INFO("Window title set to: {0}", title);
}

void GeneralEngineSettings::SetWindowSize(uint32_t width, uint32_t height)
{
    if (width > 0 && height > 0)
    {
        m_WindowWidth = width;
        m_WindowHeight = height;
        TE_CORE_INFO("Window size set to: {0}x{1}", width, height);
    }
    else
    {
        TE_CORE_WARN("Invalid window dimensions: {0}x{1}", width, height);
    }
}

void GeneralEngineSettings::SetWindowResizable(bool resizable)
{
    m_WindowResizable = resizable;
    TE_CORE_INFO("Window resizable {0}", resizable ? "enabled" : "disabled");
}

void GeneralEngineSettings::SetWindowFullscreen(bool fullscreen)
{
    m_WindowFullscreen = fullscreen;
    TE_CORE_INFO("Window fullscreen {0}", fullscreen ? "enabled" : "disabled");
}

void GeneralEngineSettings::SetWindowMaximized(bool maximized)
{
    m_WindowMaximized = maximized;
    TE_CORE_INFO("Window maximized {0}", maximized ? "enabled" : "disabled");
}

// ===== Rendering Settings =====
void GeneralEngineSettings::SetClearColor(float r, float g, float b, float a)
{
    m_ClearColor[0] = r;
    m_ClearColor[1] = g;
    m_ClearColor[2] = b;
    m_ClearColor[3] = a;
    TE_CORE_INFO("Clear color set to: ({0}, {1}, {2}, {3})", r, g, b, a);
}

void GeneralEngineSettings::SetDepthTest(bool enabled)
{
    m_DepthTest = enabled;
    TE_CORE_INFO("Depth test {0}", enabled ? "enabled" : "disabled");
}

void GeneralEngineSettings::SetBlending(bool enabled)
{
    m_Blending = enabled;
    TE_CORE_INFO("Blending {0}", enabled ? "enabled" : "disabled");
}

void GeneralEngineSettings::SetCulling(bool enabled)
{
    m_Culling = enabled;
    TE_CORE_INFO("Culling {0}", enabled ? "enabled" : "disabled");
}

void GeneralEngineSettings::SetMultisampling(bool enabled)
{
    m_Multisampling = enabled;
    TE_CORE_INFO("Multisampling {0}", enabled ? "enabled" : "disabled");
}

// ===== Input Settings =====
void GeneralEngineSettings::SetMouseSensitivity(float sensitivity)
{
    if (sensitivity > 0.0f)
    {
        m_MouseSensitivity = sensitivity;
        TE_CORE_INFO("Mouse sensitivity set to: {0}", sensitivity);
    }
    else
    {
        TE_CORE_WARN("Invalid mouse sensitivity: {0}. Must be greater than 0.", sensitivity);
    }
}

void GeneralEngineSettings::SetKeyboardRepeat(bool enabled)
{
    m_KeyboardRepeat = enabled;
    TE_CORE_INFO("Keyboard repeat {0}", enabled ? "enabled" : "disabled");
}

void GeneralEngineSettings::SetMouseAcceleration(bool enabled)
{
    m_MouseAcceleration = enabled;
    TE_CORE_INFO("Mouse acceleration {0}", enabled ? "enabled" : "disabled");
}

// ===== Audio Settings =====
void GeneralEngineSettings::SetAudioEnabled(bool enabled)
{
    m_AudioEnabled = enabled;
    TE_CORE_INFO("Audio {0}", enabled ? "enabled" : "disabled");
}

void GeneralEngineSettings::SetAudioVolume(float volume)
{
    if (volume >= 0.0f && volume <= 1.0f)
    {
        m_AudioVolume = volume;
        TE_CORE_INFO("Audio volume set to: {0}", volume);
    }
    else
    {
        TE_CORE_WARN("Invalid audio volume: {0}. Must be between 0.0 and 1.0.", volume);
    }
}

void GeneralEngineSettings::SetAudioSampleRate(uint32_t sampleRate)
{
    if (sampleRate > 0)
    {
        m_AudioSampleRate = sampleRate;
        TE_CORE_INFO("Audio sample rate set to: {0} Hz", sampleRate);
    }
    else
    {
        TE_CORE_WARN("Invalid audio sample rate: {0}. Must be greater than 0.", sampleRate);
    }
}

void GeneralEngineSettings::SetAudioChannels(uint32_t channels)
{
    if (channels >= 1 && channels <= 8)
    {
        m_AudioChannels = channels;
        TE_CORE_INFO("Audio channels set to: {0}", channels);
    }
    else
    {
        TE_CORE_WARN("Invalid audio channels: {0}. Must be between 1 and 8.", channels);
    }
}

// ===== File I/O Settings =====
void GeneralEngineSettings::SetAssetPath(const TEString &path)
{
    m_AssetPath = path;
    TE_CORE_INFO("Asset path set to: {0}", path);
}

void GeneralEngineSettings::SetConfigPath(const TEString &path)
{
    m_ConfigPath = path;
    TE_CORE_INFO("Config path set to: {0}", path);
}

void GeneralEngineSettings::SetLogPath(const TEString &path)
{
    m_LogPath = path;
    TE_CORE_INFO("Log path set to: {0}", path);
}

void GeneralEngineSettings::SetSavePath(const TEString &path)
{
    m_SavePath = path;
    TE_CORE_INFO("Save path set to: {0}", path);
}

// ===== Utility Methods =====
void GeneralEngineSettings::LoadFromFile(const TEString &filename)
{
    if (!TEFileSystem::Exists(filename))
    {
        TE_CORE_WARN("Failed to open settings file: {0}", filename);
        return;
    }

    TEFileSystem::ForEachLine(filename,
                              [this](const TEString &line)
                              {
                                  // Simple key=value parsing
                                  int pos = line.Find("=");
                                  if (pos != -1)
                                  {
                                      TEString key = line.Left(pos).Trim();
                                      TEString value = line.Mid(pos + 1).Trim();

                                      // Parse settings based on key
                                      if (key == "TargetFrameRate")
                                      {
                                          SetTargetFrameRate(value.ToFloat());
                                      }
                                      else if (key == "UnlimitedFrameRate")
                                      {
                                          SetUnlimitedFrameRate(value == "true" || value == "1");
                                      }
                                      else if (key == "VSync")
                                      {
                                          SetVSync(value == "true" || value == "1");
                                      }
                                      else if (key == "LogLevel")
                                      {
                                          SetLogLevel(value);
                                      }
                                      else if (key == "WindowWidth")
                                      {
                                          m_WindowWidth = (uint32_t)value.ToInt();
                                      }
                                      else if (key == "WindowHeight")
                                      {
                                          m_WindowHeight = (uint32_t)value.ToInt();
                                      }
                                      else if (key == "AudioVolume")
                                      {
                                          SetAudioVolume(value.ToFloat());
                                      }
                                      else if (key == "MuteAudio")
                                      {
                                          SetAudioEnabled(!(value == "true" || value == "1"));
                                      }
                                  }
                                  return true;
                              });

    TE_CORE_INFO("Engine settings loaded from: {0}", filename);
}

void GeneralEngineSettings::SaveToFile(const TEString &filename)
{
    std::ofstream file(filename);
    if (!file.is_open())
    {
        TE_CORE_ERROR("Failed to create settings file: {0}", filename);
        return;
    }

    file << "# TimeEngine Configuration File\n";
    file << "TargetFrameRate=" << m_TargetFrameRate << "\n";
    file << "UnlimitedFrameRate=" << (m_UnlimitedFrameRate ? "true" : "false") << "\n";
    file << "VSync=" << (m_VSyncEnabled ? "true" : "false") << "\n";
    file << "LogLevel=" << m_LogLevel << "\n";
    file << "WindowWidth=" << m_WindowWidth << "\n";
    file << "WindowHeight=" << m_WindowHeight << "\n";
    file << "AudioVolume=" << m_AudioVolume << "\n";

    TE_CORE_INFO("Engine settings saved to: {0}", filename);
}

void GeneralEngineSettings::ResetToDefaults()
{
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

void GeneralEngineSettings::ApplySettings()
{
    // Apply window settings
    if (Application::Get().GetWindow().GetWidth() != m_WindowWidth ||
        Application::Get().GetWindow().GetHeight() != m_WindowHeight)
    {
        TE_CORE_INFO("Window size change requires application restart");
    }

    // Apply VSync setting
    SetVSync(m_VSyncEnabled);

    // Apply logging settings
    if (m_LogToFile)
    {
        TE_CORE_INFO("File logging enabled: {0}", m_LogFile);
    }

    if (m_LogToConsole)
    {
        TE_CORE_INFO("Console logging enabled");
    }

    TE_CORE_INFO("Engine settings applied successfully");
}

// ===== Validation =====
bool GeneralEngineSettings::ValidateSettings() const { return GetValidationErrors().empty(); }

TEString GeneralEngineSettings::GetValidationErrors() const
{
    TEString errors;

    if (m_TargetFrameRate <= 0.0f)
    {
        errors += "Target frame rate must be greater than 0. ";
    }

    if (m_FrameRateLimit <= 0.0f)
    {
        errors += "Frame rate limit must be greater than 0. ";
    }

    if (m_WindowWidth == 0 || m_WindowHeight == 0)
    {
        errors += "Window dimensions must be greater than 0. ";
    }

    if (m_AudioVolume < 0.0f || m_AudioVolume > 1.0f)
    {
        errors += "Audio volume must be between 0.0 and 1.0. ";
    }

    if (m_AudioSampleRate == 0)
    {
        errors += "Audio sample rate must be greater than 0. ";
    }

    if (m_AudioChannels == 0 || m_AudioChannels > 8)
    {
        errors += "Audio channels must be between 1 and 8. ";
    }

    return errors;
}

// ===== Private Methods =====
void GeneralEngineSettings::InitializeDefaultLogCategories()
{
    m_LogCategories.Clear();
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

void GeneralEngineSettings::ValidateFrameRateSettings()
{
    if (m_TargetFrameRate > m_FrameRateLimit)
    {
        TE_CORE_WARN("Target frame rate ({0}) is higher than frame rate limit ({1})", m_TargetFrameRate,
                     m_FrameRateLimit);
    }
}

void GeneralEngineSettings::ValidateLogSettings()
{
    if (!m_LogToFile && !m_LogToConsole)
    {
        TE_CORE_WARN("Both file and console logging are disabled");
    }
}

void GeneralEngineSettings::ValidatePerformanceSettings()
{
    if (m_MaxDrawCalls == 0)
    {
        TE_CORE_WARN("Max draw calls is set to 0");
    }
    if (m_MaxTriangles == 0)
    {
        TE_CORE_WARN("Max triangles is set to 0");
    }
    if (m_MaxVertices == 0)
    {
        TE_CORE_WARN("Max vertices is set to 0");
    }
}
