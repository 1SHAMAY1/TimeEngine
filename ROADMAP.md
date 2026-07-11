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
- [ ] **Sample Projects**:
  - Build sample templates (such as Side-Scroller and Top-Down starter projects) to validate system integrations.

---

## 🔴 Long-Term Infrastructure (Missing Systems)

### 1. Editor UI Logging & Console
- [ ] **Logger Filter Control**:
  - Expose customizable logger controls (filter by severity/category) inside the **Editor Settings** panel.
- [ ] **Output Log Window**:
  - Integrate a live GUI logger window displaying core and client logs natively.
- [ ] **Built-in Editor Terminal**:
  - Integrate a command-line console interface directly inside the workspace window.

### 2. Audio Engine (Completely Missing)
- [ ] **OpenAL/FMOD Integration**:
  - Develop the `AudioWorld` manager and write wrappers for spatial sound propagation.
  - Create `AudioSourceComponent` and `AudioListenerComponent` ECS systems.

### 3. Time Manipulation & Snapshots
- [ ] **Determinism**:
  - Separate variable rendering frame loops from the fixed-timestep physics updates (`PhysicsWorld::Step`).
- [ ] **State Rollbacks**:
  - Develop component snapshot buffers and delta-compression for saving/restoring historical entity positions.
  - Implement frame-rewinding playback API controls.

### 4. Cross-Platform Runtime Layout
- [/] **Cross-Platform Porting Layer**:
  - Replace Windows-specific DLL module handling (`LoadLibraryW` / `FreeLibrary`) inside `PluginManager.cpp` with platform-agnostic abstractions (almost done via Premake build setup).
