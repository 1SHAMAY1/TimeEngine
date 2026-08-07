# Engine Settings Subsystem Architecture

The Engine Settings subsystem in TimeEngine provides global engine state management ([`EngineSettings`](file:///e:/TimeEngine/Engine/Include/Core/EngineSettings.hpp)), performance limits, logging configuration, and ImGui overlay integration (`EngineSettingsLayer` & `EngineSettingsButtonLayer`).

> [!NOTE]
> In short, think of **EngineSettings** as the engine's master control dashboard: `EngineSettings` singleton stores target framerates, VSync toggles, logging categories/levels, renderer clear colors, input sensitivity, and audio volume, while `EngineSettingsLayer` presents an immediate-mode GUI tabbed dialog inside the editor.

---

## Core Features & Settings Categories

### 1. Frame Rate & VSync Control
- Target frame rate setting (`SetTargetFrameRate`)
- Unlimited frame rate toggle (`SetUnlimitedFrameRate`)
- VSync control (`SetVSync`)

### 2. Logging Configuration
- Console & file logging toggles (`SetLogToFile`, `SetLogToConsole`)
- Log level control (`DEBUG`, `INFO`, `WARNING`, `ERROR`, `CRITICAL`)
- Category-based logging filters & custom log file naming

### 3. Performance & Limits
- Maximum draw call limiters
- Triangle & vertex cap limits
- Texture & shader pool limits

### 4. Audio & Input Settings
- Master volume control & audio toggles
- Mouse sensitivity, acceleration, & key repeat rates

---

## Core Classes & Component Roles

1. **[`TE::EngineSettings`](file:///e:/TimeEngine/Engine/Include/Core/EngineSettings.hpp)**:
   - Singleton class storing global settings parameters.
   - Provides getter/setter API for performance limits, VSync, logging levels, window resolution, audio, and asset paths.
   - Persists settings to disk via `SaveToFile(path)` and `LoadFromFile(path)`.

2. **[`TE::EngineSettingsLayer`](file:///e:/TimeEngine/Engine/Include/Layers/EngineSettingsLayer.hpp)**:
   - ImGui overlay tabbed dialog allowing real-time modification of engine settings inside the editor workspace.

3. **[`TE::EngineSettingsButtonLayer`](file:///e:/TimeEngine/Engine/Include/Layers/EngineSettingsButtonLayer.hpp)**:
   - Floating viewport overlay button (`⚙️ Engine Settings`) toggling `EngineSettingsLayer`.

---

## Usage Guidelines & Code Snippets

### 1. Singleton Configuration API

```cpp
#include "Core/EngineSettings.hpp"

// Configure global engine limits
EngineSettings& settings = EngineSettings::Get();
settings.SetTargetFrameRate(120.0f);
settings.SetVSync(true);
settings.SetLogLevel("INFO");
settings.ApplySettings();
```

---

### 2. Settings Validation & File Operations

```cpp
EngineSettings& settings = EngineSettings::Get();

// Validate configuration parameters
if (!settings.ValidateSettings()) {
    TE_CORE_ERROR("Settings validation failed: {0}", settings.GetValidationErrors());
}

// Save / Load engine settings file
settings.SaveToFile("Config/EngineSettings.json");
settings.LoadFromFile("Config/EngineSettings.json");

// Reset configuration to factory defaults
settings.ResetToDefaults();
```

---

## Related Architectural Documentation

- [Core Subsystem Architecture](file:///e:/TimeEngine/Engine/src/Core/ARCHITECTURE.md) — Main engine application execution loop.
- [Multi-Threading & Task System Architecture](file:///e:/TimeEngine/Engine/Include/Core/Threading/ARCHITECTURE.md) — Worker thread pools and task queues.
- [Layers Subsystem Architecture](file:///e:/TimeEngine/Engine/src/Core/Layers/ARCHITECTURE.md) — `EngineSettingsLayer` and overlay rendering.
- [Root Architecture Index](file:///e:/TimeEngine/ARCHITECTURE.md) — Master TimeEngine architecture index.
