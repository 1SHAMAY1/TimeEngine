# EngineSettings System

The EngineSettings system provides a comprehensive way to control global engine settings including frame rate, logging options, performance limits, and more.

## Overview

The EngineSettings system consists of two main components:

1. **EngineSettings** - The core settings management class (singleton)
2. **EngineSettingsLayer** - UI layer for managing settings through ImGui
3. **EngineSettingsButtonLayer** - Button layer that provides a toggle button for the settings UI

## Features

### Frame Rate Control
- Target frame rate setting
- Unlimited frame rate option
- VSync control
- Frame rate limiting

### Logging Configuration
- File and console logging toggles
- Log level control (DEBUG, INFO, WARNING, ERROR, CRITICAL)
- Category-based logging filters
- Timestamp options
- Custom log file naming

### Performance Settings
- Maximum draw calls limit
- Maximum triangles limit
- Maximum vertices limit
- Maximum textures limit
- Maximum shaders limit

### Debug Options
- Debug mode toggle
- FPS display
- Performance metrics display
- Debug info display
- Wireframe rendering
- Bounding box display

### Window Settings
- Window title
- Window size
- Resizable option
- Fullscreen mode
- Maximized state

### Rendering Settings
- Clear color
- Depth testing
- Blending
- Face culling
- Multisampling

### Input Settings
- Mouse sensitivity
- Keyboard repeat
- Mouse acceleration

### Audio Settings
- Audio enable/disable
- Volume control
- Sample rate
- Channel count

### File I/O Settings
- Asset path
- Config path
- Log path
- Save path

## Usage

### Basic Usage

```cpp
#include "Core/EngineSettings.hpp"

// Get the singleton instance
EngineSettings& settings = EngineSettings::Get();

// Set frame rate
settings.SetTargetFrameRate(60.0f);
settings.SetVSync(true);

// Configure logging
settings.SetLogToFile(true);
settings.SetLogLevel("INFO");

// Apply settings
settings.ApplySettings();
```

### Using the UI Layer

```cpp
#include "Layers/EngineSettingsButtonLayer.hpp"

// Create and add the button layer
TE::EngineSettingsButtonLayer* settingsButton = new TE::EngineSettingsButtonLayer();
application->PushLayer(settingsButton);
```

### Settings Validation

```cpp
EngineSettings& settings = EngineSettings::Get();

// Validate settings
if (!settings.ValidateSettings()) {
    std::string errors = settings.GetValidationErrors();
    TE_CORE_ERROR("Settings validation failed: {0}", errors);
}
```

### File Operations

```cpp
EngineSettings& settings = EngineSettings::Get();

// Save settings to file
settings.SaveToFile("my_settings.json");

// Load settings from file
settings.LoadFromFile("my_settings.json");

// Reset to defaults
settings.ResetToDefaults();
```

## UI Components

### EngineSettingsButtonLayer
- Provides a floating button to toggle the settings window
- Positioned at (10, 150) by default
- Shows "⚙️ Engine Settings" button
- Tooltip explains functionality

### EngineSettingsLayer
- Comprehensive settings UI with tabs
- Frame Rate tab
- Logging tab
- Performance tab
- Debug tab
- Window tab
- Rendering tab
- Input tab
- Audio tab
- File I/O tab
- Settings management tab

## Integration

The EngineSettings system integrates with:

- **Application** - Window and VSync control
- **Logging System** - Log level and output control
- **Profiling System** - Performance metrics display
- **ImGui** - UI rendering and docking

## Default Values

The system provides sensible defaults:

- Target Frame Rate: 60 FPS
- VSync: Enabled
- Log Level: INFO
- Window Size: 1280x720
- Audio Volume: 1.0
- Mouse Sensitivity: 1.0
- Clear Color: Black (0, 0, 0, 1)

## Thread Safety

The EngineSettings class is designed to be thread-safe for read operations. Write operations should be performed on the main thread to ensure proper synchronization with the rendering system.

## Future Enhancements

- JSON serialization for settings files
- Hot-reloading of settings
- Profile-based settings (Debug, Release, Dist)
- Command-line argument support
- Settings inheritance and overrides 