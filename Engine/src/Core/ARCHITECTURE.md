# TimeEngine Core Subsystem Architecture

The **Core Subsystem** is the central engine hub of TimeEngine. It drives the main application execution loop (`Application`), multi-threaded worker thread pool initialization, window event callbacks, layer stack updates, immediate-mode GUI rendering passes, and coordinates all sub-engine modules.

> [!NOTE]
> In short, think of **Core** as the main brain and engine chassis of TimeEngine: `Application` instantiates the OS window, initializes logging and thread pools, boots up the dynamic plugin manager (`PluginManager`), and loops continuously—updating game logic, rendering framebuffers, handling window events, and ticking timers.

---

## Core Engine Architecture & Main Loop Pipeline

```
                              [ Application::Run() ]
                                         │
 ┌───────────────────────────────────────┴───────────────────────────────────────┐
 │                                                                               │
 ▼                                                                               ▼
[ RenderCommand::Clear() ]                                       [ LayerStack Updates ]
 (Clear Framebuffer to Black)                                     (Iterate Layer::OnUpdate)
 │                                                                               │
 ├───────────────────────────────────────────────────────────────────────────────┤
 │                                                                               │
 ▼                                                                               ▼
[ Immediate-Mode TimeGUI Pass ]                                 [ Deferred Processing ]
 (TimeGUILayer::Begin/End, OnTimeGUIRender)                      (ProcessDeferredRemovals & Additions)
 │                                                                               │
 └───────────────────────────────────────┬───────────────────────────────────────┘
                                         │
                                         ▼
                              [ m_Window->OnUpdate() ]
                              (Poll GLFW Events & Swap Buffers)
```

---

## Central Engine Subsystems (Core Directories)

Click any module link below to open its comprehensive architectural reference:

* 🎬 **[Scene & ECS Architecture](Scene/ARCHITECTURE.md)** — Entity-Component-System container (`EntityManager`), component lifecycle, JSON scene serialization (`.tescene`), and `Scene::OnUpdateRuntime`.
* ⚛️ **[2D Physics Architecture](Physics/ARCHITECTURE.md)** — Rigid body dynamics (`RigidBody`), Symplectic Euler integration, raycasting, soft body node blobs, and 2D constraint joints.
* 💥 **[2D Collision Architecture](Collision/ARCHITECTURE.md)** — Broadphase spatial pair filtering (`BroadPhase`), narrowphase Separating Axis Theorem (SAT) geometry checks (`CollisionSystem`), and collider components.
* 📦 **[Asset Subsystem Architecture](Asset/ARCHITECTURE.md)** — 64-bit `AssetHandle` mappings, path-handle registry (`AssetRegistry`), prototype metadata registration (`AssetManager`), and `stb_image` I/O.
* 🎮 **[GameFramework Architecture](GameFrameWork/ARCHITECTURE.md)** — Base reflected game object (`TObject`), 2D spatial component (`TComponent`), hierarchy nesting, mouse picking (`ContainsPoint`), 2D shadow occlusion, and `StandardGameLibrary` object pooling.
* ⚡ **[Events Subsystem Architecture](Events/ARCHITECTURE.md)** — Blocking event classes (`ApplicationEvent`, `KeyEvent`, `MouseEvent`), category bitmasks, and type-safe event dispatching (`EventDispatcher`).
* 🧵 **[Multi-Threading & Task System Architecture](../../Include/Core/Threading/ARCHITECTURE.md)** — Dedicated thread pools (`ThreadPool`, `TaskSystem`), 6 worker pools (Main, Render, Gameplay, AI, Calc, Widget), and async job macros (`SUBMIT_CALC`, `SUBMIT_AI`).
* 🥞 **[Layers Subsystem Architecture](Layers/ARCHITECTURE.md)** — Modular execution stack (`LayerStack`), regular game layers vs top-priority GUI overlays, deferred removals/additions, and event propagation pipelines.

* ⏱️ **[Core Time Architecture](Time/ARCHITECTURE.md)** — Frame delta calculation, fixed-rate metronome tickers (`Ticker`), and static delayed/looping timers (`Timer::Set`, `Timer::NextFrame`).
* 📁 **[Project Subsystem Architecture](Project/ARCHITECTURE.md)** — Active workspace configuration (`ProjectConfig`), root directory resolution, and `.teproj` text file serialization (`ProjectSerializer`).
* 📄 **[Core Public Headers & Threading Index](../../Include/Core/ARCHITECTURE.md)** — Public header interface index (`Engine/Include/Core/`).
* ⚙️ **[Engine Settings Architecture](../../Include/Core/EngineSettings_ARCHITECTURE.md)** — Singleton engine configuration (`EngineSettings`), target framerates, VSync, logging filters, and `EngineSettingsLayer`.
* 🔌 **[Plugin Manager Architecture](Plugin/ARCHITECTURE.md)** — Dynamic OS library loader (`LoadLibraryW` / `dlopen`), `.teplugin` descriptor parser, symbol factory lookups (`CreatePluginInstance`), and reverse-order unloader.

---

## Core Main Application Execution (`Application`)

### 1. `Application::Application()` (Initialization Phase)

During constructor execution, `Application` sets up the core engine subsystems in exact sequence:
1. **Logging Initialization**: Configures `TE::Log::Init(true, "TimeEngineLog.json")` for structured JSON log output.
2. **Multi-Thread Pools**: Spawns dedicated engine worker thread contexts:
   - `INIT_MAIN_THREAD()`
   - `INIT_RENDER_THREAD()`
   - `INIT_CALC_THREAD()`
   - `INIT_AI_THREAD()`
   - `INIT_WIDGET_THREAD()`
   - `INIT_GAMEPLAY_THREAD()`
3. **Window Creation**: Instantiates `m_Window = std::unique_ptr<IWindow>(IWindow::Create())` (creates GLFW native window).
4. **GLAD Graphics Loader**: Binds OpenGL function pointers via `RenderCommand::LoadLoader()`.
5. **Event Callback Hook**: Configures window callback function to handle `WindowCloseEvent`, `WindowResizeEvent`, `WindowFocusEvent`, and route remaining events down the `LayerStack` from top to bottom.
6. **GUI Overlay**: Instantiates `TimeGUILayer` and pushes it as a top overlay.
7. **Plugin Discovery**: Invokes `PluginManager::Initialize()` to load engine and project dynamic plugins.

---

### 2. `Application::Run()` (Main Engine Game Loop)

The main engine loop runs continuously until `m_Running` becomes `false`:

```cpp
void Application::Run() {
    while (m_Running) {
        // 1. Clear Framebuffer
        RenderCommand::SetClearColor(TEColor::Black());
        RenderCommand::Clear();

        // 2. Application Logic Update
        OnUpdate();

        // 3. Layer Stack Logic Updates (Bottom-to-Top)
        for (Layer *layer : m_LayerStack) {
            if (layer)
                layer->OnUpdate();
        }

#ifdef TE_EDITOR
        // 4. Immediate-Mode GUI Render Pass (Bottom-to-Top)
        m_TimeGUILayer->Begin();
        for (Layer *layer : m_LayerStack) {
            if (layer)
                layer->OnTimeGUIRender();
        }
        m_TimeGUILayer->End();
#endif

        // 5. Deferred Cleanup & Additions
        m_LayerStack.ProcessDeferredRemovals();
        ProcessDeferredAdditions();

        // 6. Swap Buffers & Poll Hardware Events
        m_Window->OnUpdate();
    }

    IWindow::Terminate();
}
```

---

### 3. Safe Layer Management (Deferred Operations)

#### `Application::MarkLayerForAddition(layer)` & `Application::MarkLayerForRemoval(layer)`
- **Why Deferred**: If a layer (or button event inside a layer) requests creating or destroying another layer during frame updates, modifying `m_LayerStack` directly would invalidate standard vector iterators and crash the engine loop.
- **Mechanism**: `MarkLayerForAddition` queues layers into `m_LayersToAdd`, and `ProcessDeferredAdditions()` flushes them at the end of the frame step after `OnTimeGUIRender()` completes.

---

## Client Entry Point Architecture (`EntryPoint.h`)

TimeEngine applications do not define a traditional `main()` function in user code. Instead, `EntryPoint.h` implements standard cross-platform entry points and delegates creation to the client via `TE::CreateApplication()`:

```cpp
// Executable entry point defined in Engine/src/Core/EntryPoint.h
int main(int argc, char** argv) {
    auto app = TE::CreateApplication(argc, argv);
    app->Run();
    delete app;
    return 0;
}
```
p after `OnTimeGUIRender()` completes.

---

## Client Entry Point Architecture ([`EntryPoint.h`](file:///e:/TimeEngine/Engine/src/Core/EntryPoint.h))

TimeEngine applications do not define a traditional `main()` function in user code. Instead, `EntryPoint.h` implements standard cross-platform entry points and delegates creation to the client via `TE::CreateApplication()`:

```cpp
// Executable entry point defined in Engine/src/Core/EntryPoint.h
int main(int argc, char** argv) {
    auto app = TE::CreateApplication(argc, argv);
    app->Run();
    delete app;
    return 0;
}
```
