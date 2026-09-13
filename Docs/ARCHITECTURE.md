# TimeEngine Architecture & Build System Overview

This document provides a concise high-level architecture index and build configuration guide for **TimeEngine**.

> [!NOTE]
> In short, **TimeEngine** is a high-performance, multi-threaded 2D/3D game engine and editor built in modern C++. It features a modular Entity-Component-System (ECS), multi-backend graphics API abstraction (OpenGL 4.5 DSA, DirectX 11, Vulkan 1.3, OpenGL ES 3.0, Metal), immediate-mode `TimeGUI` rendering, action-mapped input system, physics/collisions, and root-level marketplace plugin support (`.teplugin` / `.dll`).

---

## 🏛️ Modular Subsystem Architecture Guides

* ⚙️ **[Engine Modular Architecture](Engine/Source/ARCHITECTURE.md)** — Modular runtime subsystems (`Core`, `Math`, `RHI`, `Renderer2D`, `Physics`, `Scripting`, `Scene`, `Testing`, `UI`, `Window`) and framework layers (`Asset`, `Input`, `Gameplay`, `Application`).
  * 📁 **[Project Subsystem Architecture](Engine/Source/Framework/Application/Source/Project/ARCHITECTURE.md)** — Active workspace configuration (`ProjectConfig`), root directory resolution, and `.teproj` serialization.
  * ⚡ **[Events Subsystem Architecture](Engine/Source/Runtime/Core/Source/Events/ARCHITECTURE.md)** — Blocking event classes (`ApplicationEvent`, `KeyEvent`, `MouseEvent`) and type-safe `EventDispatcher`.
  * 🧵 **[Multi-Threading & Task System Architecture](Engine/Source/Runtime/Core/Include/Threading/ARCHITECTURE.md)** — Dedicated thread pools (`ThreadPool`, `TaskSystem`), 6 worker pools (Main, Render, Gameplay, AI, Calc, Widget), and async job macros (`SUBMIT_CALC`, `SUBMIT_AI`).
  * 🥞 **[Layers Subsystem Architecture](Engine/Source/Framework/Application/Source/Layers/ARCHITECTURE.md)** — Modular execution stack (`LayerStack`), regular game layers vs top-priority GUI overlays.
  * 🔌 **[Plugin Manager Architecture](Engine/Source/Runtime/Core/Source/Plugin/ARCHITECTURE.md)** — Dynamic OS library loader (`LoadLibraryW` / `dlopen`), `.teplugin` descriptor parser, and symbol factories.
  * 🧪 **[Automated Testing Subsystem](Engine/Source/Runtime/Testing/Include/ARCHITECTURE.md)** — Headless test runner (`HeadlessTestRunner`), unit, integration, and stress test suites.
* 🖥️ **[TimeEditor IGDE Architecture](TimeEditor/Source/ARCHITECTURE.md)** — Visual editor workspace modes, retained-mode UI component system (`UIWidget`), universal polymorphic saving (`EditorSaveManager`), layout manager (`EditorLayoutManager`), and viewport rendering.
* 🔌 **[Marketplace Plugins Architecture](Plugins/ARCHITECTURE.md)** — Dynamic OS shared library (`.dll`/`.so`) plugin slot architecture partitioned into `Shipping/` and `Experimental/` tiers.
  * 🤖 **[MCP Plugin Architecture](Plugins/Shipping/MCPPlugin/ARCHITECTURE.md)** — Model Context Protocol HTTP/SSE AI receiver plugin (`http://127.0.0.1:3000`).

---

## 🛠️ Build System (`Premake5.lua`) & Build Commands

TimeEngine uses **Premake5** ([`Premake5.lua`](Premake5.lua)) with a **Tiered Parallel Compilation Pipeline** outputting all binaries to `Artifacts/Bin/` and `Artifacts/Bin-Intermediate/`:

```
Tier 1: ThirdParty Libraries (GLM, GLFW, Velox, ForgeUI, Logger)
   ↓
Tier 2: PCH & Core Types (PreRequisites.h, TECore, TEMath)
   ↓
Tier 3: Abstract Interfaces (TERHI, IWindow, UIAPI, IPlugin)
   ↓
Tier 4: Concrete Backends & Subsystems in Parallel (OpenGL, Vulkan, DX11, GLES, Metal, TEScene, TEScripting, TETesting)
   ↓
Tier 5: TimeEditor & Marketplace Plugins in Parallel
```

### Building on Windows (MSVC)
```bat
:: Generate Visual Studio 2022 project files
Scripts\Windows\GenerateProjectFiles.bat

:: Compile x64 Debug build via multi-threaded MSBuild
Scripts\Windows\MSVC\BuildDebug.bat
```

### Building on Linux / Unix (GCC / Clang)
```bash
# 1. Setup submodules & dependencies (installs required packages + mold linker)
bash Scripts/Linux/SetupSubmodules.sh

# 2. Generate workspace Makefiles:
# GCC toolchain:
bash Scripts/Linux/GCC/GenerateProjectFiles.sh
# OR Clang toolchain:
bash Scripts/Linux/Clang/GenerateProjectFiles.sh

# 3. Build the engine and editor:
# GCC Debug build:
bash Scripts/Linux/GCC/BuildDebug.sh
# OR Clang Debug build:
bash Scripts/Linux/Clang/BuildDebug.sh
```

---

## 📜 Automation & Tooling Scripts (`Scripts/`)

* 🧪 **[`Scripts/Windows/RunTests.bat`](Scripts/Windows/RunTests.bat)** — Runs headless unit and integration test suites.
* 💥 **[`Scripts/Windows/RunStressTests.bat`](Scripts/Windows/RunStressTests.bat)** — Multi-threaded memory and entity stress testing harness.
* 📦 **[`Scripts/Linux/SetupSubmodules.sh`](Scripts/Linux/SetupSubmodules.sh)** / **[`Scripts/Windows/SetupSubmodules.bat`](Scripts/Windows/SetupSubmodules.bat)** — Automated dependency validator, submodule updater, and Premake downloader.
* 🐧 **[`Scripts/Linux/GCC/`](Scripts/Linux/GCC/)** & **[`Scripts/Linux/Clang/`](Scripts/Linux/Clang/)** — Dedicated Linux build scripts with multithreaded compilation and fast `mold`/`lld` linking.
* 🪟 **[`Scripts/Windows/MSVC/`](Scripts/Windows/MSVC/)**, **[`Scripts/Windows/Clang/`](Scripts/Windows/Clang/)**, **[`Scripts/Windows/MinGW/`](Scripts/Windows/MinGW/)** — Dedicated Windows build scripts.
* 🍏 **[`Scripts/Mac/Makefiles/`](Scripts/Mac/Makefiles/)** & **[`Scripts/Mac/Xcode/`](Scripts/Mac/Xcode/)** — Dedicated macOS build suites.
* 🔗 **[`Scripts/Linux/GCC/RegisterFileExtension.sh`](Scripts/Linux/GCC/RegisterFileExtension.sh)** / **[`Scripts/Windows/RegisterFileExtension.bat`](Scripts/Windows/RegisterFileExtension.bat)** — MIME type and OS file association registration utilities.

---

## 🔒 ThirdParty Insulation & Core Engine Types Policy

Direct calls to third-party vendor APIs outside their designated engine wrapper layers and raw unmanaged memory allocations are strictly prohibited:

| Subsystem / Type | Wrapper Layer / Convention | Class / Header |
| :--- | :--- | :--- |
| **Smart Pointers** | `Scope<T>` (`CreateScope`), `Ref<T>` (`CreateRef`), `WeakRef<T>` | [`PreRequisites.h`](Engine/Source/Runtime/Core/Include/PreRequisites.h) |
| **Engine String** | `TEString`, `TEStringView` (No raw `std::string` in engine APIs) | [`TEString.hpp`](Engine/Source/Runtime/Core/Include/TEString.hpp) |
| **Containers & Views** | `TEArray<T>`, `TEOption<T>`, `TESpan<T>`, `TEResult<T, E>` | [`GameplayUtils.hpp`](Engine/Source/Framework/Gameplay/Include/GameplayUtils.hpp) |
| **Math & Vectors** | `TEVector2`, `TEVector`, `TEVector4`, `TEMatrix4`, `TEQuat` | [`MathUtils.hpp`](Engine/Source/Runtime/Math/Include/MathUtils.hpp) |
| **UI Engine** | `TimeGUI` (Strict UI abstraction) | [`TimeGUI.hpp`](Engine/Source/Runtime/UI/Include/TimeGUI.hpp) |
| **Windowing** | `IWindow` | [`IWindow.hpp`](Engine/Source/Runtime/Window/Include/IWindow.hpp) |
| **Physics** | `PhysicsWorld` | [`PhysicsWorld.hpp`](Engine/Source/Runtime/Physics/Include/PhysicsWorld.hpp) |

---

## 📄 Further References

* 📄 **[ROADMAP.md](ROADMAP.md)** — Feature milestones and deterministic time-manipulation engine plans.
* 🤖 **[llms.txt](llms.txt)** / **[llms.md](llms.md)** — Architectural reference files for AI coding assistants.
