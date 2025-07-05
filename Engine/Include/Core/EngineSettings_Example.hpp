#pragma once

/**
 * EngineSettings Usage Examples
 * 
 * This file demonstrates how to use the EngineSettings class to control
 * global engine settings like frame rate and logging options.
 */

#include "Core/EngineSettings.hpp"
#include "Core/Log.h"

namespace TE {

    /**
     * Example class showing how to use EngineSettings
     */
    class EngineSettingsExample {
    public:
        static void DemonstrateFrameRateSettings() {
            // Get the singleton instance
            EngineSettings& settings = EngineSettings::Get();
            
            // Set frame rate settings
            settings.SetTargetFrameRate(60.0f);
            settings.SetUnlimitedFrameRate(false);
            settings.SetVSync(true);
            settings.SetFrameRateLimit(120.0f);
            
            TE_CORE_INFO("Frame rate settings configured");
        }
        
        static void DemonstrateLoggingSettings() {
            EngineSettings& settings = EngineSettings::Get();
            
            // Configure logging
            settings.SetLogToFile(true);
            settings.SetLogToConsole(true);
            settings.SetLogLevel("INFO");
            settings.SetLogTimestamp(true);
            settings.SetLogFile("MyAppLog.json");
            
            // Enable/disable specific log categories
            settings.SetLogCategory("Renderer", true);
            settings.SetLogCategory("Audio", false);
            settings.SetLogCategory("Network", true);
            
            TE_CORE_INFO("Logging settings configured");
        }
        
        static void DemonstratePerformanceSettings() {
            EngineSettings& settings = EngineSettings::Get();
            
            // Set performance limits
            settings.SetMaxDrawCalls(5000);
            settings.SetMaxTriangles(500000);
            settings.SetMaxVertices(1000000);
            settings.SetMaxTextures(500);
            settings.SetMaxShaders(50);
            
            TE_CORE_INFO("Performance settings configured");
        }
        
        static void DemonstrateDebugSettings() {
            EngineSettings& settings = EngineSettings::Get();
            
            // Configure debug options
            settings.SetDebugMode(true);
            settings.SetShowFPS(true);
            settings.SetShowPerformanceMetrics(true);
            settings.SetShowDebugInfo(false);
            settings.SetShowWireframe(false);
            settings.SetShowBoundingBoxes(true);
            
            TE_CORE_INFO("Debug settings configured");
        }
        
        static void DemonstrateWindowSettings() {
            EngineSettings& settings = EngineSettings::Get();
            
            // Configure window properties
            settings.SetWindowTitle("My Awesome Game");
            settings.SetWindowSize(1920, 1080);
            settings.SetWindowResizable(true);
            settings.SetWindowFullscreen(false);
            settings.SetWindowMaximized(false);
            
            TE_CORE_INFO("Window settings configured");
        }
        
        static void DemonstrateRenderingSettings() {
            EngineSettings& settings = EngineSettings::Get();
            
            // Configure rendering options
            settings.SetClearColor(0.1f, 0.1f, 0.1f, 1.0f);
            settings.SetDepthTest(true);
            settings.SetBlending(true);
            settings.SetCulling(true);
            settings.SetMultisampling(true);
            
            TE_CORE_INFO("Rendering settings configured");
        }
        
        static void DemonstrateInputSettings() {
            EngineSettings& settings = EngineSettings::Get();
            
            // Configure input options
            settings.SetMouseSensitivity(1.5f);
            settings.SetKeyboardRepeat(true);
            settings.SetMouseAcceleration(false);
            
            TE_CORE_INFO("Input settings configured");
        }
        
        static void DemonstrateAudioSettings() {
            EngineSettings& settings = EngineSettings::Get();
            
            // Configure audio options
            settings.SetAudioEnabled(true);
            settings.SetAudioVolume(0.8f);
            settings.SetAudioSampleRate(48000);
            settings.SetAudioChannels(2);
            
            TE_CORE_INFO("Audio settings configured");
        }
        
        static void DemonstrateFileIOSettings() {
            EngineSettings& settings = EngineSettings::Get();
            
            // Configure file paths
            settings.SetAssetPath("game/assets/");
            settings.SetConfigPath("game/config/");
            settings.SetLogPath("game/logs/");
            settings.SetSavePath("game/saves/");
            
            TE_CORE_INFO("File I/O settings configured");
        }
        
        static void DemonstrateSettingsManagement() {
            EngineSettings& settings = EngineSettings::Get();
            
            // Validate settings
            if (!settings.ValidateSettings()) {
                TE_CORE_ERROR("Settings validation failed: {0}", settings.GetValidationErrors());
                return;
            }
            
            // Apply all settings
            settings.ApplySettings();
            
            // Save settings to file
            settings.SaveToFile("engine_settings.json");
            
            TE_CORE_INFO("Settings management completed");
        }
        
        static void DemonstrateCompleteSetup() {
            TE_CORE_INFO("=== Engine Settings Complete Setup ===");
            
            // Configure all settings
            DemonstrateFrameRateSettings();
            DemonstrateLoggingSettings();
            DemonstratePerformanceSettings();
            DemonstrateDebugSettings();
            DemonstrateWindowSettings();
            DemonstrateRenderingSettings();
            DemonstrateInputSettings();
            DemonstrateAudioSettings();
            DemonstrateFileIOSettings();
            
            // Apply and save
            DemonstrateSettingsManagement();
            
            TE_CORE_INFO("=== Engine Settings Setup Complete ===");
        }
        
        static void DemonstrateSettingsQuery() {
            EngineSettings& settings = EngineSettings::Get();
            
            TE_CORE_INFO("=== Current Engine Settings ===");
            TE_CORE_INFO("Target Frame Rate: {0} FPS", settings.GetTargetFrameRate());
            TE_CORE_INFO("VSync Enabled: {0}", settings.IsVSyncEnabled() ? "Yes" : "No");
            TE_CORE_INFO("Log Level: {0}", settings.GetLogLevel());
            TE_CORE_INFO("Debug Mode: {0}", settings.IsDebugModeEnabled() ? "Enabled" : "Disabled");
            TE_CORE_INFO("Window Size: {0}x{1}", settings.GetWindowWidth(), settings.GetWindowHeight());
            TE_CORE_INFO("Audio Volume: {0}", settings.GetAudioVolume());
            TE_CORE_INFO("Mouse Sensitivity: {0}", settings.GetMouseSensitivity());
            TE_CORE_INFO("Asset Path: {0}", settings.GetAssetPath());
            TE_CORE_INFO("=== End Settings Query ===");
        }
    };

} // namespace TE 