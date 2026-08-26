# Engine Settings Subsystem Architecture

The Engine Settings subsystem in TimeEngine provides an extensible developer settings hierarchy ([`EngineSettings`](EngineSettings.hpp)), an automated registration registry ([`EngineSettingsRegistry`](EngineSettingsRegistry.hpp)), and concrete core settings ([`GeneralEngineSettings`](GeneralEngineSettings.hpp)).

> [!NOTE]
> In short, think of **EngineSettings** as the engine's modular configuration architecture: developers and plugin authors derive custom settings classes from `EngineSettings`, register them via `TE_REGISTER_SETTINGS(MySettingsClass)`, and access them globally via `EngineSettingsRegistry::GetSettings<MySettingsClass>()`.

---

## Core Classes & Component Roles

1. **[`TE::EngineSettings`](EngineSettings.hpp)**:
   - Base settings interface defining category metadata, serialization hooks, and validation methods.
   - Provides virtual methods:
     - `GetCategoryName()` (e.g. `"Engine"`, `"Project"`, `"Editor"`, `"Plugins"`)
     - `GetSectionName()` (e.g. `"General"`, `"Rendering"`, `"Physics"`)
     - `GetDisplayName()`, `GetDescription()`
     - `OnInitialized()`, `OnModified()`, `ResetToDefaults()`, `ValidateSettings()`
     - `Save(configDir)`, `Load(configDir)`

2. **[`TE::EngineSettingsRegistry`](EngineSettingsRegistry.hpp)**:
   - Central singleton managing instances of all registered `EngineSettings` subclasses.
   - Automatically instantiates and registers classes at static init time.
   - Provides type-safe retrieval:
     ```cpp
     TERef<MyCustomSettings> settings = EngineSettingsRegistry::GetSettings<MyCustomSettings>();
     ```

3. **[`TE_REGISTER_SETTINGS(Class)`](EngineSettingsRegistry.hpp)**:
   - Declarative registration macro that binds a developer settings subclass into the engine hierarchy.

4. **[`TE::GeneralEngineSettings`](GeneralEngineSettings.hpp)**:
   - Concrete subclass for core engine settings (framerate, VSync, logging, performance caps, windowing, audio, clear color).
   - Singleton convenience accessor: `GeneralEngineSettings::Get()`.

---

## Creating & Registering Custom Developer Settings

### 1. Define Custom Settings Class

```cpp
#include "Core/Settings/EngineSettings.hpp"
#include "Core/Settings/EngineSettingsRegistry.hpp"

class GameplaySettings : public TE::EngineSettings
{
public:
    GameplaySettings() = default;
    virtual ~GameplaySettings() = default;

    virtual std::string GetCategoryName() const override { return "Project"; }
    virtual std::string GetSectionName() const override { return "Gameplay"; }
    virtual std::string GetDisplayName() const override { return "Gameplay & World Rules"; }

    // Custom configuration properties
    float PlayerGravity = 9.8f;
    int MaxEnemySpawns = 64;
    bool bEnableFriendlyFire = false;

    virtual void ResetToDefaults() override
    {
        PlayerGravity = 9.8f;
        MaxEnemySpawns = 64;
        bEnableFriendlyFire = false;
    }
};

// Register for automatic discovery and instantiation
TE_REGISTER_SETTINGS(GameplaySettings);
```

---

### 2. Accessing Settings Anywhere in Engine / Game Code

```cpp
#include "Core/Settings/EngineSettingsRegistry.hpp"

// Query typed settings instance
auto gameplaySettings = TE::EngineSettingsRegistry::GetSettings<GameplaySettings>();
if (gameplaySettings)
{
    float gravity = gameplaySettings->PlayerGravity;
}
```

---

### 3. Core Engine Configuration (`GeneralEngineSettings`)

```cpp
#include "Core/Settings/GeneralEngineSettings.hpp"

// Access global general settings
auto &general = TE::GeneralEngineSettings::Get();
general.SetTargetFrameRate(120.0f);
general.SetVSync(true);
general.SetLogLevel("INFO");
general.ApplySettings();
```

---

## Related Architectural Documentation

- [Core Subsystem Architecture](../../src/Core/ARCHITECTURE.md) — Main engine application execution loop.
- [Multi-Threading Subsystem Architecture](../Threading/ARCHITECTURE.md) — Worker thread pools and task queues.
- [Layers Subsystem Architecture](../../src/Core/Layers/ARCHITECTURE.md) — `EngineSettingsLayer` and overlay UI.
- [Root Architecture Index](../../../ARCHITECTURE.md) — Master TimeEngine architecture index.
