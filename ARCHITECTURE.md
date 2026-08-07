# TimeEngine Architecture & Build System Overview

This document provides a concise high-level architecture index and build configuration guide for **TimeEngine**.

> [!NOTE]
> In short, **TimeEngine** is a multi-threaded 2D C++17 game engine. It uses Premake5 for cross-platform build generation, modular vendor insulation wrappers, and automated CLI scripts.

---

## 🏛️ Subsystem Architecture Guides

* ⚙️ **[Engine Subsystem Architecture](Engine/ARCHITECTURE.md)** — Core engine loop, ECS, 2D physics, 2D collisions, asset manager, and multi-backend renderers (OpenGL 4.5, DirectX 11, Vulkan, OpenGLES).
  * 📁 **[Project Subsystem Architecture](file:///e:/TimeEngine/Engine/src/Core/Project/ARCHITECTURE.md)** — Active workspace config (`ProjectConfig`), root directory resolution, and `.teproj` serialization.
  * ⚙️ **[Engine Settings Architecture](file:///e:/TimeEngine/Engine/Include/Core/EngineSettings_ARCHITECTURE.md)** — Singleton engine settings (`EngineSettings`), target framerates, VSync, logging filters, and UI layer.
  * ⚡ **[Events Subsystem Architecture](file:///e:/TimeEngine/Engine/src/Core/Events/ARCHITECTURE.md)** — Blocking event classes (`ApplicationEvent`, `KeyEvent`, `MouseEvent`) and type-safe `EventDispatcher`.
  * 🧵 **[Multi-Threading & Task System Architecture](file:///e:/TimeEngine/Engine/Include/Core/Threading/ARCHITECTURE.md)** — Dedicated thread pools (`ThreadPool`, `TaskSystem`), 6 worker pools (Main, Render, Gameplay, AI, Calc, Widget), and async job macros (`SUBMIT_CALC`, `SUBMIT_AI`).
  * 🥞 **[Layers Subsystem Architecture](file:///e:/TimeEngine/Engine/src/Core/Layers/ARCHITECTURE.md)** — Modular execution stack (`LayerStack`), regular game layers vs top-priority GUI overlays.
  * 🔌 **[Plugin Manager Architecture](file:///e:/TimeEngine/Engine/src/Core/Plugin/ARCHITECTURE.md)** — Dynamic OS library loader (`LoadLibraryW` / `dlopen`), `.teplugin` descriptor parser, and symbol factories.
* 📁 **[Engine Source Directory Index](Engine/src/ARCHITECTURE.md)** — Navigation index for `Engine/src/` subdirectories (`Core`, `Renderer`, `Editor`, `Input`, `Camera`, `Window`, `Utils`).
* 🖥️ **[TimeEditor IGDE Architecture](TimeEditor/ARCHITECTURE.md)** — Visual editor workspace modes, asset inspectors, toolbar controls, and viewport rendering.
* 🔌 **[Plugins Architecture](Engine/Plugins/ARCHITECTURE.md)** — Dynamic OS shared library (`.dll`/`.so`) plugin slot architecture.
* 🤖 **[MCP Plugin Architecture](Engine/Plugins/MCPPlugin/ARCHITECTURE.md)** — Model Context Protocol HTTP/SSE AI receiver plugin (`http://127.0.0.1:3000`).

---

## 🛠️ Build System (`Premake5.lua`) & Build Commands

TimeEngine uses **Premake5** ([`Premake5.lua`](file:///e:/TimeEngine/Premake5.lua)) to generate native Visual Studio solution files (`.sln`), Makefiles, or Ninja projects across Windows, Linux, and macOS.

### Building on Windows (MSVC / MSBuild)
```bash
# Generate Visual Studio 2022 project files
Vendor/Premake/premake5.exe vs2022

# Compile x64 Debug build via MSBuild
MSBuild.exe Engine/Engine.vcxproj /p:Configuration=Debug /p:Platform=x64
```

### Building on Linux / Unix (GNU Make)
```bash
# Generate Makefiles
./Vendor/Premake/premake5 gmake2

# Compile with GCC/Clang
make -j$(nproc)
```

---

## 📜 Automation & Tooling Scripts (`Scripts/`)

* 🛠️ **[`Scripts/MCP_Tools.sh`](file:///e:/TimeEngine/Scripts/MCP_Tools.sh)** — CLI helper script mapping command-line tool calls to JSON-RPC 2.0 payloads for the embedded MCP server.
* 🐍 **[`Scripts/parse_json.py`](file:///e:/TimeEngine/Scripts/parse_json.py)** — Python helper for constructing complex JSON entity mutation payloads.
* 📦 **[`Scripts/SetupSubmodules.sh`](file:///e:/TimeEngine/Scripts/SetupSubmodules.sh)** — Automated Git submodule dependency fetcher.
* 🪟 **[`Scripts/Windows/CleanProjectFiles.bat`](file:///e:/TimeEngine/Scripts/Windows/CleanProjectFiles.bat)** — Batch utility cleaning generated binaries (`Bin/`, `Bin-Intermediate/`).
* 🔗 **[`Scripts/Windows/RegisterFileExtension.bat`](file:///e:/TimeEngine/Scripts/Windows/RegisterFileExtension.bat)** — Win32 script registering `.teproj` and `.tescene` file associations in Windows Registry.

---

## 🔒 Vendor Insulation Policy

To ensure long-term portability and maintainability, direct calls to third-party vendor APIs outside their designated engine wrapper layers are strictly prohibited:

| Vendor Library | Wrapper Layer | Class / Header |
| :--- | :--- | :--- |
| **ImGui** | `TimeGUI` | [`TimeGUI.hpp`](file:///e:/TimeEngine/Engine/Include/Utils/TimeGUI.hpp) |
| **GLM** | `MathUtils` | [`MathUtils.hpp`](file:///e:/TimeEngine/Engine/Include/Utils/MathUtils.hpp) |
| **GLFW** | `IWindow` | [`IWindow.hpp`](file:///e:/TimeEngine/Engine/Include/Window/IWindow.hpp) |
| **Velox / Physics** | `PhysicsWorld` | [`PhysicsWorld.hpp`](file:///e:/TimeEngine/Engine/Include/Core/Physics/PhysicsWorld.hpp) |

---

## 📄 Further References

* 📄 **[ROADMAP.md](ROADMAP.md)** — Feature milestones and deterministic time-manipulation engine plans.
* 🤖 **[llms.txt](llms.txt)** / **[llms.md](llms.md)** — Architectural reference file for AI coding assistants.
