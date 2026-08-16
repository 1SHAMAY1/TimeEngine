# Plugins Architecture

This directory contains optional and dynamic extensions (plugins) for TimeEngine.

> [!NOTE]
> In short, imagine TimeEngine as a high-tech gaming console. Instead of building every single feature directly into the main console hardware, the **Plugin System** acts as a "universal extension slot". Any new feature (like an AI bridge, custom editor tools, or networked controller) can be plugged in or unplugged without modifying or rebuilding the core console.

---

## Plugin Function Roles & Usage Guidelines

### 1. `OnLoad()`
- **When Used**: Called by the engine immediately after the dynamic library (`.dll`/`.so`) is loaded into memory.
- **What to do inside**: Spawn background threads, start HTTP/network servers (e.g. MCP server), initialize assets, or subscribe to engine event buses.
- **When NOT to use**: Do NOT perform heavy, blocking startup work directly on the main thread here if it delays engine startup.

```cpp
void MCPPlugin::OnLoad() {
    m_Running = true;
    m_ServerThread = std::thread(&MCPPlugin::ServerThreadMain, this);
}
```

### 2. `OnUnload()`
- **When Used**: Called by the engine right before the dynamic library is detached or shut down.
- **What to do inside**: Gracefully close open socket handles, signal background threads to exit (`m_Running = false`), join threads, and free allocated memory.
- **When NOT to use**: Do NOT throw exceptions or block indefinitely inside `OnUnload()`.

```cpp
void MCPPlugin::OnUnload() {
    m_Running = false;
    if (m_ServerThread.joinable())
        m_ServerThread.join();
}
```

### 3. Factory Exports (`CreatePluginInstance` & `DestroyPluginInstance`)
- **`CreatePluginInstance()`**: Called by the engine's plugin loader to instantiate the plugin on the heap.
- **`DestroyPluginInstance(plugin)`**: Called by the engine loader to safely delete the plugin instance using the plugin's own heap allocator.
- **Preferred Rule**: Always pair dynamic allocation with `DestroyPluginInstance` to prevent cross-DLL heap allocation crashes on Windows.

---

## Registered Plugins & Architectures

- [MCPPlugin Architecture](MCPPlugin/ARCHITECTURE.md) — Model Context Protocol (MCP) HTTP/SSE Server Plugin architecture documentation.
- [GameplayTagPlugin Architecture](GameplayTagPlugin/ARCHITECTURE.md) — Hierarchical Gameplay Tags Plugin architecture documentation.


