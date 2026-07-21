# TimeEngine Development Roadmap

This document outlines the current state and missing features of TimeEngine, mapped directly to active components and architectures found in the codebase.

---

## 🟢 Completed Milestones (Production-Ready)

### 1. Multi-API Renderer & Windowing Backend
- [x] **Batched 2D Renderer (`Renderer2D`)**: High-performance rendering pipeline with backends for OpenGL 4.5, Vulkan, OpenGL ES, and DirectX 11.
- [x] **Component Drawers**: Automatic rendering of standard shapes (`BoxComponent`, `CircleComponent`, `TriangleComponent`).
- [x] **Strict Vendor Abstractions (Wrappers)**: Direct usage of third-party libraries (ImGui via `TimeGUI`, GLFW via `IWindow`/`WindowsWindow`, and GLM via `MathUtils`) isolated to dedicated wrapper classes.

### 2. Serialization & File Management
- [x] **Scene Serialization (`SceneSerializer`)**: Full YAML-based scene state saving and loading.
- [x] **Asset Manager**: Dynamic management of asset types and directory creation.

### 3. Editor & Plugin Systems
- [x] **Plugins Settings Panel**: Runtime plugin management via `Edit -> Plugins` using `PluginManager` for dynamic DLL loading/unloading.
- [x] **MCP Automation Server**: Remote JSON-RPC interface over Server-Sent Events (SSE) on port 3000.

---

## 🟡 Short-Term Goals (Next Steps)

### 1. ECS Component & Gameplay Framework
- [/] **`ParallaxComponent` Integration**:
  - Integrate existing `ParallaxComponent` (in [ParallaxComponent.hpp](file:///e:/TimeEngine/Engine/Include/Core/Scene/ParallaxComponent.hpp)) into `Renderer2D` rendering passes.
  - Implement property inspectors for depth speed settings inside `TEPropertyDrawer`.
- [ ] **`AnimatedSpriteComponent`**:
  - Implement full flipbook render loop integration into the batched renderer.
- [ ] **Gameplay Framework Expansion**:
  - Complete the player base controller stubs ([PlayerControllerBase.hpp](file:///e:/TimeEngine/Engine/Include/Core/Scene/PlayerControllerBase.hpp) and [2DPlayerController.hpp](file:///e:/TimeEngine/Engine/Include/Core/Scene/2DPlayerController.hpp)).
  - Implement gameplay logic systems, level trigger volumes, game loop managers, and dynamic script state bindings.
- [ ] **Tilemap & Level Design Core System**:
  - Auto-Tiling Brush: A tilemap painting system with custom brushes and layer support.
  - Dynamic Physics Colliders: Automatically compiles individual tile colliders into optimized static compound collision shapes inside the `PhysicsWorld` to prevent collision snagging.
- [ ] **Scripting Bindings Core System**:
  - Lua/Wren Integration: Embeds a fast, lightweight scripting language.
  - State-Safe Sandbox: Keeps all script variables in structured tables that can be serialized/deserialized instantly for time rollbacks, ensuring script logic remains perfectly deterministic.
- [ ] **Behavior Tree & AI Core System**:
  - Visual Node Graph: Allows developers to link Selector, Sequence, Decorator, and Action nodes visually.
  - Real-time Debugger: Highlights the active execution path during gameplay simulation (turns nodes green/red based on success/failure) to easily debug NPC behavior.
  - Task Registrator: Scans the compiled gameplay code or scripting boundaries and registers gameplay action tasks as visual graph nodes automatically.
- [ ] **VFX & Particle Designer Core System**:
  - Emitter Node Graph: Control particle lifetimes, velocity curves, gravity modifiers, color gradients, and size over time using visual curves.
  - VFX Playback Panel: Scrub, pause, and preview particles in real-time in the editor viewport without starting the main game loop.

### 2. Physics Engine Integration (Velox)
- [ ] **Extended Collision Shapes**:
  - Expose Triangle and Polygon colliders in `PhysicsWorld::AddBody` (mapped from [TriangleComponent.hpp](file:///e:/TimeEngine/Engine/Include/Core/Scene/TriangleComponent.hpp)).
- [ ] **Velocity Syncing**:
  - Improve real-time feedback loop when setting entity velocity components directly inside properties panels.

### 3. Tooling & Automation
- [/] **Sprite & Asset Editors (Early Stages)**:
  - Add programmatic controls for shape lists (`m_VectorElements`), paint canvas grids, and math scripting shader buffers (`m_ProcBuffer`).
  - Upgrade primitive asset inspectors to full graphical asset editing editors.
- [/] **Cross-Platform Developer Automation**:
  - Symmetric script suites for Windows, Linux, and macOS to clean files, generate project workspaces via Premake, register OS file associations, and run production builds (Note: Only `CleanProjectFiles` and `GenerateProjectFiles` have been tested on Windows; all other scripts, as well as Linux and macOS environments, remain untested).
  - Expose unified cross-platform tool wrappers (`Scripts/MCP_Tools.sh`) for remote scripting and debugging.
- [ ] **2D PCG (Procedural Content Generation) Toolset Library Plugin**:
  - Implement node-based coordinates and layout generation pipeline using `PCGPointData` (Grid, Random/Scatter, Poisson Disk, and Path/Line generators).
  - Implement noise and terrain nodes (Perlin/Simplex/Worley noise, Elevation mapping, and Slope operators).
  - Implement structural creators (Wilson's maze, Cellular Automata, BSP rooms, Drunkard's Walk, and Wave Function Collapse solver).
  - Develop 12 specialized ECS-compatible processor nodes:
    1. *Voronoi Partitioning Node* (biome and faction territory generator)
    2. *Delaunay Triangulation Node* (corridor linker graph)
    3. *Physics Point Relaxer* (XPBD soft spacing solver)
    4. *Raycast Projection Node* (snapping points to physics surfaces/slopes)
    5. *Bounds Exclusion Volume Node* (area influence masking)
    6. *ECS Component Attribute Injector* (injecting point data to component properties)
    7. *Entity Assembler & Hierarchy Linker* (creating parent-child hierarchies)
    8. *L-System Structural Generator* (fractal branching coordinates)
    9. *Edge/Border Extractor Node* (isolating perimeter points)
    10. *Density-Weighted Clustering (K-Means) Node* (finding camps/points of interest)
    11. *Pathfinding Traversal (A\*) Generator* (road/bridge linker path)
    12. *Component Proximity Mutator* (distance-based scaling of attributes)
- [ ] **Localization & Translation Tooling Plugin**:
  - Implement spreadsheet-style dictionary grids and dynamic text length visualizers.
  - Develop in-editor live locale switchers for real-time translation previewing.
  - Implement CSV/JSON export and import pipeline compiler scripts (`.telocale`).
  - Develop ECS integrations: `LocalizedTextComponent` dynamic updating, locale font override system, pluralization token handlers, and currency/number formatters.
- [ ] **Sprite Sheet & Atlas Packager Plugin**:
  - Implement the MaxRects bin packing layout engine with aspect ratio and Power-of-Two width/height constraints.
  - Develop transparent border trimming offsets and pixel extrusion dilation settings to eliminate texture bleeding seams.
  - Implement coordinate mapping JSON exporters and asset registers for `.teatlas` engine assets.
  - Integrate atlas loaders into `AnimatedSpriteComponent` properties with dynamic sequence tag parsing.
- [ ] **Sample Projects**:
  - Build sample templates (such as Side-Scroller and Top-Down starter projects) to validate system integrations.

---

## 🔴 Long-Term Infrastructure (Missing Systems)

### 1. Editor UI Logging & Console
- [x] **Logger Filter Control**:
  - Expose customizable logger controls (filter by severity/category) inside the **Editor Settings** panel.
- [x] **Output Log Window**:
  - Integrate a live GUI logger window displaying core and client logs natively.
- [x] **Built-in Editor Terminal**:
  - Integrate a command-line console interface directly inside the workspace window.

### 2. Audio Engine (Completely Missing)
- [ ] **OpenAL/FMOD Integration**:
  - Develop the `AudioWorld` manager and write wrappers for spatial sound propagation.
  - Create `AudioSourceComponent` and `AudioListenerComponent` ECS systems.
- [ ] **Pitch-to-Time Dilation Link**:
  - Automatically modulates the audio playback speed and pitch in real-time to match the engine's current time dilation factor (e.g., audio pitches down when time slows down).

### 3. Time Manipulation & Snapshots
- [ ] **Determinism**:
  - Separate variable rendering frame loops from the fixed-timestep physics updates (`PhysicsWorld::Step`).
- [ ] **State Rollbacks**:
  - Develop component snapshot buffers and delta-compression for saving/restoring historical entity positions.
  - Implement frame-rewinding playback API controls.
- [ ] **Deterministic Netcode / Multiplayer Rollback System**:
  - Rollback Sync: Implements a client-side prediction and rollback network framework using the engine's time rewind capabilities. If a packet conflict occurs, the engine rolls back to the conflict frame, runs the new inputs, and fast-forwards back to the current frame in one step.

### 4. Cross-Platform Build & Runtime Support

> **Current State**: TimeEngine is built exclusively with **MSVC (Visual Studio 2022)** on Windows. Shell scripts for Linux and macOS exist under `Scripts/Linux/` and `Scripts/Mac/` but are untested and incomplete. No MinGW support exists. The source platform layer only contains `Engine/src/Platform/Windows/`.

#### Platform Status

| Platform | Compiler | IDE/Build | Status |
|---|---|---|---|
| Windows (MSVC) | MSVC | Visual Studio 2022 `.sln` | ✅ Primary / Working |
| Linux (GCC/Clang) | GCC or Clang | Premake → `gmake2` | ⚠️ Scripts exist, untested |
| macOS (AppleClang) | AppleClang | Premake → `xcode4` | ⚠️ Scripts exist, untested |
| Windows (MinGW) | GCC/MinGW | Premake → `gmake2` | ❌ Not set up |

#### Required Changes (Premake5.lua)
- [ ] Guard MSVC-only compiler flags (`/utf-8`, `/FS`, `/Z7`) under `filter "toolset:msc"` — they break GCC/Clang.
- [ ] Guard DirectX links (`d3d11.lib`, `dxgi.lib`, `d3dcompiler.lib`) under `filter "system:windows and toolset:msc"` — unavailable on Linux/macOS.
- [ ] Replace `opengl32.lib` with platform-conditional links:
  - Linux: `-lGL`, `-ldl`, `-lpthread` (via X11/Wayland through GLFW)
  - macOS: `-framework OpenGL`, `-framework Cocoa`, `-framework IOKit`, `-framework CoreVideo`
- [ ] Guard `xcopy` post-build commands under `filter "system:windows"` — use `cp`/`install` equivalents for Linux/macOS.
- [ ] Add `filter "system:linux"` and `filter "system:macosx"` source file includes for platform-specific `.cpp` implementations.
- [ ] Add MinGW support: new `Scripts/Windows/GenerateProjectFiles_MinGW.bat` targeting `gmake2`.

#### Required Changes (Source Code)
- [ ] **`PluginManager.cpp`**: Replace `LoadLibraryW` / `FreeLibrary` / `GetProcAddress` (Windows-only) with `dlopen` / `dlclose` / `dlsym` on Linux/macOS, guarded by `TE_PLATFORM_*` defines.
- [ ] **`Engine/src/Platform/Linux/`**: Implement Linux platform layer (window system, input handling, timer/clock, dynamic library loader).
- [ ] **`Engine/src/Platform/Mac/`**: Implement macOS platform layer (Cocoa windowing via GLFW, input, timer, `dlopen`-based plugin loader).
- [ ] **Shared library output**: `.dll` (Windows) → `.so` (Linux) → `.dylib` (macOS). Premake handles naming; ensure runtime loaders use the correct extension per platform.
- [ ] **Preprocessor defines**: Ensure `TE_PLATFORM_WINDOWS` / `TE_PLATFORM_LINUX` / `TE_PLATFORM_MAC` are correctly set and guarded throughout the codebase.

#### macOS-Specific Considerations
- [ ] OpenGL is **deprecated on macOS 10.14+**. Long-term renderer options: **MoltenVK** (Vulkan-over-Metal), native **Metal** backend, or **OpenGL ES** via ANGLE — OpenGL ES is already included as a vendor (`Vendor/OpenGL-Registry/api`) and is a viable fallback target.
- [ ] `icon` field in Premake uses `.ico` (Windows only) — add `.icns` asset and conditional icon assignment.

#### CI/CD (GitHub Actions)

All jobs use `fail-fast: false` so every target reports independently. Use `aminya/setup-cpp` for toolchain bootstrapping and `ccache` for GCC/Clang build caching.

| Target | Runner | Toolchain | Generator | Status |
|---|---|---|---|---|
| Windows — MSVC x64 | `windows-latest` | MSVC 2022 (`ilammy/msvc-dev-cmd`) | Premake → `vs2022` | ✅ Already in CI |
| Windows — Clang-cl x64 | `windows-latest` | LLVM Clang-cl (MSVC-compat mode) | Premake → `vs2022` | ❌ Not set up |
| Windows — MinGW-w64 | `windows-latest` | GCC via MinGW-w64 | Premake → `gmake2` | ❌ Not set up |
| Linux — GCC | `ubuntu-latest` | GCC 13+ (`apt install g++`) | Premake → `gmake2` | ❌ Not set up |
| Linux — Clang | `ubuntu-latest` | LLVM Clang 17+ | Premake → `gmake2` | ❌ Not set up |
| macOS — AppleClang x64 | `macos-13` | Xcode AppleClang | Premake → `gmake2` or `xcode4` | ❌ Not set up |
| macOS — AppleClang ARM64 | `macos-14` | Xcode AppleClang (Apple Silicon) | Premake → `gmake2` or `xcode4` | ❌ Not set up |
| WebAssembly (Emscripten) | `ubuntu-latest` | Emscripten SDK (`mymindstorm/setup-emsdk`) | CMake + Emscripten toolchain | ❌ Not set up |
| Android (NDK) | `ubuntu-latest` | Android NDK r25+ (built-in on runner) | CMake + `android.toolchain.cmake` | ❌ Not set up |

**Notes per target:**
- **Clang-cl**: Drop-in MSVC replacement on Windows; catches MSVC-specific code portability issues early.
- **MinGW-w64**: Most common free Windows toolchain; widely used in open-source C++ projects and tools like MSYS2.
- **Linux GCC + Clang**: Both should be tested — GCC catches strict aliasing issues, Clang catches UB and sanitizer warnings.
- **macOS ARM64** (`macos-14`): Apple Silicon (M1/M2); all new Macs ship ARM64. Must be a separate CI job from Intel (`macos-13`).
- **Emscripten/WASM**: Enables browser-based preview/demo builds. Requires renderer to compile headlessly (OpenGL ES via WebGL2 path).
- **Android NDK**: Enables mobile porting. Uses `TE_PLATFORM_ANDROID` define; no D3D, no desktop GL — OpenGL ES only.
- **iOS**: Subset of macOS ARM64 job using Xcode simulator target; lower priority, depends on Metal/MoltenVK renderer decision.

#### Prioritized Implementation Order
1. Fix `Premake5.lua` compiler/OS guards *(unblocks everything below, non-breaking for Windows)*.
2. Add `Platform/Linux/` source layer + test Linux build in CI.
3. Refactor `PluginManager` to use `dlopen`/`dlclose` on non-Windows.
4. Add MinGW support on Windows (`gmake2` + MinGW toolchain).
5. Add `Platform/Mac/` source layer and resolve Metal/MoltenVK renderer direction.
