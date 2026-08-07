# Dynamic Plugin Manager Architecture

The `PluginManager` module (`Core/Plugin/PluginManager.hpp` & `PluginManager.cpp`) provides discovery, descriptor parsing (`.teplugin`), dynamic OS library loading (`LoadLibraryW` / `dlopen`), symbol resolution (`GetProcAddress` / `dlsym`), and lifecycle management for TimeEngine plugins.

> [!NOTE]
> In short, think of the **PluginManager** as the engine's dynamic software installer: it scans engine/project directories for `.teplugin` manifest files, loads shared libraries (`.dll`/`.so`) into memory at runtime, calls `CreatePluginInstance()`, triggers `OnLoad()`, and safely unloads them in reverse order during shutdown.

---

## Plugin Discovery & Metadata Structure

### `.teplugin` Descriptor Format
Plugins are identified by `.teplugin` plain-text descriptor files located in `Engine/Plugins/` or `<ProjectDir>/Plugins/`:

```yaml
Name: MCPPlugin
Version: 1.0.0
Description: Model Context Protocol HTTP/SSE Server Plugin
Enabled: true
```

### `PluginInfo` Struct
- `Name`: String identifier of the plugin.
- `Version` / `Description`: Metadata displayed in Editor settings.
- `Enabled`: Boolean state (persisted to `.teplugin`).
- `Path`: Filesystem path to `.teplugin` descriptor file.
- `LibraryPath`: Resolved path to compiled shared library (`.dll`, `.so`, or `.dylib`).

---

## Core Functions & Execution Rules

### 1. `PluginManager::Initialize()`
- **When Used**: Invoked during application startup by `Application::Application()`.
- **What it does**: Calls `DiscoverPlugins()` to scan engine and project plugin directories, parses descriptors, and automatically calls `LoadPlugin()` for all enabled plugins.

---

### 2. `PluginManager::LoadPlugin(pluginDescriptorPath)`
- **When Used**: Called when loading a specific discovered plugin or when a user enables a plugin in settings.
- **Workflow**:
  1. Parses `.teplugin` descriptor file (`ParsePluginDescriptor`).
  2. Loads OS dynamic library (`LoadLibraryW` on Windows / `dlopen` on Unix).
  3. Resolves factory symbol `CreatePluginInstance` via `GetProcAddress` / `dlsym`.
  4. Calls `CreatePluginInstance()` to instantiate `TE::IPlugin`.
  5. Pushes `LoadedPluginInstance` record and calls `instance->OnLoad()`.

```cpp
// Explicitly loading a plugin by descriptor path
TE::PluginManager::LoadPlugin("Engine/Plugins/MCPPlugin/MCPPlugin.teplugin");
```

---

### 3. `PluginManager::UnloadPlugin(name)`
- **When Used**: Called when disabling a plugin or unloading plugins during project switch.
- **Workflow**:
  1. Locates `LoadedPluginInstance` matching `name`.
  2. Invokes `Instance->OnUnload()`.
  3. Resolves and calls `DestroyPluginInstance` export symbol to safely delete instance.
  4. Unloads OS shared library (`FreeLibrary` on Windows / `dlclose` on Unix).

---

### 4. `PluginManager::SetPluginEnabled(name, enabled)`
- **When Used**: Called when toggling plugin checkboxes in Editor Settings UI.
- **Behavior**: Rewrites `Enabled: true/false` key inside the target `.teplugin` file and dynamically calls `LoadPlugin()` or `UnloadPlugin()`.

---

### 5. `PluginManager::Shutdown()`
- **When Used**: Called during engine termination inside `Application::~Application()`.
- **Behavior**: Unloads all active plugins in **reverse order of loading** to honor system dependencies.

---

## Related Architectural Documentation

- [Engine Plugins Architecture](file:///e:/TimeEngine/Engine/Plugins/ARCHITECTURE.md) — Architecture documentation for optional engine plugins and dynamic plugin authoring.

