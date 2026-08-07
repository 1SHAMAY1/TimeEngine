# TimeEngine Master Architecture & Subsystem Directory

This document serves as the high-level master architectural index for **TimeEngine**, detailing the engine runtime directory structure, core execution pipeline, and providing direct navigation links to all subsystem architectural documentation across the codebase.

> [!NOTE]
> In short, **TimeEngine** is a high-performance 2D/3D game engine and editor built in C++17. It features a multi-threaded architecture (6 dedicated worker thread pools), modular Entity-Component-System (ECS) scene graph, multi-backend graphics API abstraction (OpenGL 4.5 DSA, DirectX 11, Vulkan 1.3, OpenGL ES 3.0), immediate-mode `TimeGUI` rendering, action-mapped input system, physics/collisions, and dynamic OS shared-library plugin support (`.teplugin` / `.dll`).

---

## 🏛️ Top-Level Engine Structure

```
TimeEngine/
├── Engine/
│   ├── Include/                  # Vendor-insulated public header files & interfaces
│   ├── Plugins/                  # Engine dynamic plugins (MCPPlugin remote AI receiver)
│   ├── src/                      # Engine C++ source code & subsystem implementations
│   │   ├── Camera/               # 2D/3D projection camera math & controllers
│   │   ├── Core/                 # Engine main loop, thread pools, ECS, physics, asset manager
│   │   ├── Editor/               # Visual editor workspace modes & asset tab inspectors
│   │   ├── Input/                # High-level action mapping context system
│   │   ├── Renderer/             # Multi-API renderer backends (OpenGL, DX11, Vulkan, OpenGLES)
│   │   ├── Utils/                # Engine toolbox (MathUtils, TimeGUI, Win32/POSIX OS platform)
│   │   └── Window/               # OS native window creation (GLFW/Win32) & swap chain
```

---

## 🗺️ Master Subsystem Architecture Index

Each link below opens a detailed, dedicated architectural reference document:

### 1. Engine Core Subsystems
* 🏠 **[Engine Source Directory Index](file:///e:/TimeEngine/Engine/src/ARCHITECTURE.md)** — Master index for all `Engine/src/` subdirectories.
* ⚙️ **[Core Subsystem Architecture](file:///e:/TimeEngine/Engine/src/Core/ARCHITECTURE.md)** — Main engine loop (`Application`), thread pools, ECS, physics, assets, events, layer stack, and plugins.
  * 🎬 **[Scene & ECS Architecture](file:///e:/TimeEngine/Engine/src/Core/Scene/ARCHITECTURE.md)** — Entity lifespan (`EntityManager`), components, `.tescene` serialization, and runtime playback.
  * ⚛️ **[2D Physics Architecture](file:///e:/TimeEngine/Engine/src/Core/Physics/ARCHITECTURE.md)** — Rigid body dynamics (`RigidBody`), Symplectic Euler integration, 2D joints, and soft bodies.
  * 💥 **[2D Collision Architecture](file:///e:/TimeEngine/Engine/src/Core/Collision/ARCHITECTURE.md)** — Broadphase spatial pair filtering (`BroadPhase`) and SAT narrowphase overlap checks (`CollisionSystem`).
  * 📦 **[Asset Subsystem Architecture](file:///e:/TimeEngine/Engine/src/Core/Asset/ARCHITECTURE.md)** — 64-bit `AssetHandle` mappings, path registry (`AssetRegistry`), prototype metadata, and image I/O.
  * 🎮 **[GameFramework Architecture](file:///e:/TimeEngine/Engine/src/Core/GameFrameWork/ARCHITECTURE.md)** — Reflected base object (`TObject`), spatial component (`TComponent`), hierarchy, picking, and object pooling.
  * ⚡ **[Events Subsystem Architecture](file:///e:/TimeEngine/Engine/src/Core/Events/ARCHITECTURE.md)** — Blocking event classes (`ApplicationEvent`, `KeyEvent`, `MouseEvent`) and type-safe `EventDispatcher`.
  * 🥞 **[Layers Subsystem Architecture](file:///e:/TimeEngine/Engine/src/Core/Layers/ARCHITECTURE.md)** — Modular execution stack (`LayerStack`), regular game layers vs top-priority GUI overlays.
  * 🎮 **[Core Low-Level Input Architecture](file:///e:/TimeEngine/Engine/src/Core/Input/ARCHITECTURE.md)** — Hardware polling (`Input`), key hold duration timers (`InputState`), and scroll accumulators.
  * ⏱️ **[Core Time Architecture](file:///e:/TimeEngine/Engine/src/Core/Time/ARCHITECTURE.md)** — Frame delta calculation, fixed metronome tickers (`Ticker`), and static delayed timers (`Timer`).
  * 📁 **[Project Subsystem Architecture](file:///e:/TimeEngine/Engine/src/Core/Project/ARCHITECTURE.md)** — Active workspace config (`ProjectConfig`), root directory resolution, and `.teproj` serialization.
  * 🔌 **[Plugin Manager Architecture](file:///e:/TimeEngine/Engine/src/Core/Plugin/ARCHITECTURE.md)** — Dynamic OS library loader (`LoadLibraryW` / `dlopen`), `.teplugin` descriptor parser, and symbol factories.

### 2. Graphics & Rendering Subsystems
* 🎨 **[Renderer Subsystem Architecture](file:///e:/TimeEngine/Engine/src/Renderer/ARCHITECTURE.md)** — 2D batching pipeline (`Renderer2D`, `RenderBatcher`), 2D lights, shadow volumes, materials, and shaders.
  * 🟢 **[OpenGL Backend Architecture](file:///e:/TimeEngine/Engine/src/Renderer/OpenGL/ARCHITECTURE.md)** — Cross-platform desktop OpenGL 4.5 DSA hardware renderer implementation.
  * 🔷 **[DirectX 11 Backend Architecture](file:///e:/TimeEngine/Engine/src/Renderer/DirectX11/ARCHITECTURE.md)** — Direct3D 11 hardware backend implementation (`DX11Context`, D3DCompile HLSL shaders).
  * 📱 **[OpenGL ES Backend Architecture](file:///e:/TimeEngine/Engine/src/Renderer/OpenGLES/ARCHITECTURE.md)** — Embedded & mobile OpenGL ES 3.0 hardware backend implementation.
  * 🌋 **[Vulkan Backend Architecture](file:///e:/TimeEngine/Engine/src/Renderer/Vulkan/ARCHITECTURE.md)** — Explicit Vulkan 1.3 low-overhead hardware backend implementation (`volk`, SPIR-V).
  * 🎨 **[Material System Architecture](file:///e:/TimeEngine/Engine/src/Renderer/Material/ARCHITECTURE.md)** — Node-based material pass stack (`MaterialPassNode`), uniform caching, and `.tematerial` serialization.

### 3. Engine Tools, Viewport & Windowing
* 🛠️ **[Editor Subsystem Architecture](file:///e:/TimeEngine/Engine/src/Editor/ARCHITECTURE.md)** — Visual editor workspace modes (`EditorModeRegistry`), specialized asset inspectors (`AssetEditorRegistry`), and toolbars.
* 🎯 **[High-Level Input Architecture](file:///e:/TimeEngine/Engine/src/Input/ARCHITECTURE.md)** — Action mapping contexts (`InputSystem`, `InputAction`, `InputMappingContext`), priority contexts, and `InputComponent`.
* 📷 **[Camera Subsystem Architecture](file:///e:/TimeEngine/Engine/src/Camera/ARCHITECTURE.md)** — 2D/3D camera matrix calculation (`Camera`), `OrthographicCamera`, `PerspectiveCamera`, and `PlayerCameraComponent`.
* 🖥️ **[Window Subsystem Architecture](file:///e:/TimeEngine/Engine/src/Window/ARCHITECTURE.md)** — Native window abstraction (`IWindow`, `WindowsWindow`), GLFW callbacks, VSync, and swap chain buffers.
* 🧰 **[Utils Subsystem Architecture](file:///e:/TimeEngine/Engine/src/Utils/ARCHITECTURE.md)** — Core engine toolbox, 2D engine math primitives (`MathUtils`), vendor-agnostic GUI wrappers (`TimeGUI`), and OS platform utilities.
  * 🪟 **[Win32 Platform Architecture](file:///e:/TimeEngine/Engine/src/Utils/Platform/Windows/ARCHITECTURE.md)** — Win32 file dialogs (`IFileDialog`), HKCU Windows Registry associations, and executable path resolution.
  * 🐧 **[Unix Platform Architecture](file:///e:/TimeEngine/Engine/src/Utils/Platform/Unix/ARCHITECTURE.md)** — POSIX executable path resolution (`/proc/self/exe`), native Unix dialogs, and MIME registrations.

### 4. Engine Plugins
* 🔌 **[Plugins Subsystem Architecture](file:///e:/TimeEngine/Engine/Plugins/ARCHITECTURE.md)** — Engine dynamic plugin slot architecture and dynamic extension loading guidelines.
  * 🤖 **[MCP Plugin Architecture](file:///e:/TimeEngine/Engine/Plugins/MCPPlugin/ARCHITECTURE.md)** — Model Context Protocol HTTP/SSE server plugin listening on port `3000` for remote AI engine automation.
