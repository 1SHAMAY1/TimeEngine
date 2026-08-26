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
  - Integrate existing `ParallaxComponent` (in [ParallaxComponent.hpp](Engine/Include/Core/Scene/ParallaxComponent.hpp)) into `Renderer2D` rendering passes.
  - Implement property inspectors for depth speed settings inside `TEPropertyDrawer`.
- [ ] **`AnimatedSpriteComponent`**:
  - Implement full flipbook render loop integration into the batched renderer.
- [ ] **Gameplay Framework Expansion**:
  - Complete the player base controller stubs ([PlayerControllerBase.hpp](Engine/Include/Core/Scene/PlayerControllerBase.hpp) and [2DPlayerController.hpp](Engine/Include/Core/Scene/2DPlayerController.hpp)).
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
  - Expose Triangle and Polygon colliders in `PhysicsWorld::AddBody` (mapped from [TriangleComponent.hpp](Engine/Include/Core/Scene/TriangleComponent.hpp)).
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

### 3. Editor Play-In-Editor (PIE) & Standalone Runtime Architecture (Phase 1)
- [ ] **Play/Pause/Stop PIE Loop (`Engine/src/Core/Layers/EditorLayer.hpp` & `.cpp`)**:
  - **In-Memory Scene Copying**: When hitting **Play**, deep-copy `m_EditorScene` via `Scene::Copy()` into `m_RuntimeScene` (in `Engine/src/Core/Scene/Scene.cpp`). Runtime mutations (physics, script variables, entity destructions) occur exclusively on `m_RuntimeScene`, leaving `m_EditorScene` pristine.
  - **State Machine Integration**: Manage `SceneState m_SceneState` (`Edit`, `Play`, `Pause`). `OnPlay()` executes `m_RuntimeScene->OnRuntimeStart()`; `OnPause()` freezes `OnUpdateRuntime(dt)` and physics steps while maintaining the active viewport for step debugging; `OnStop()` calls `OnRuntimeStop()`, clears `m_RuntimeScene`, and restores `m_EditorScene`.
  - **3-State Toolbar UI (`Engine/src/Editor/EditorToolbar.cpp`)**: Add Play ▶, Pause ⏸, Resume ▶, and Stop ⏹ GUI buttons with visual state toggles using `TimeGUI::ImageButton`.
  ```cpp
  // Editor state machine snippet
  void EditorLayer::OnPlay() {
      m_RuntimeScene = Scene::Copy(m_EditorScene);
      m_RuntimeScene->OnRuntimeStart();
      m_SceneState = SceneState::Play;
  }
  ```
- [ ] **Standalone Process Launching (`Engine/src/Core/Layers/GameLayer.hpp` & `.cpp`)**:
  - **Game-Only CLI Launch**: Add `▶ Standalone` toolbar launcher that executes `PlatformUtils::LaunchProcess()` targeting `TimeEditor.exe` with CLI arguments: `--standalone --project=<path> --scene=<path>` (`Engine/src/Core/EntryPoint.h`).
  - **GameLayer Execution**: Implement a lightweight `GameLayer` that skips editor ImGui dockspaces and tool panels, rendering active `.tescene` files full-screen for production-accurate GPU performance, raw input capture, and window resolution testing.

---

### 4. TScript Language Engine (C/Python Hybrid, Zero External Compiler) (Phase 2)
- [x] **Parser & AST Architecture (Zero Binary Dependencies)**:
  - **Lexer & Pratt Parser (`Engine/Include/Core/Scripting/TScriptLexer.hpp` & `TScriptParser.hpp`)**: Hand-crafted single-pass lexer tokenizing identifiers, literals, operators, and keywords into a recursive-descent Pratt parser building a typed AST (`TScriptAST.hpp`).
  - **AST Caching Engine (`Engine/Include/Core/Scripting/TScriptAsset.hpp`)**: Parse `.tscript` source text **once** on save/edit and cache the resulting AST (`TScriptProgram`) inside `TScriptAsset`. The tree-walk interpreter (`TScriptInterpreter.hpp`) walks the cached AST per frame — eliminating string re-parsing and avoiding raw binary disk files that can corrupt.
- [x] **Hybrid Language Features & Engine Bindings**:
  - **C/Python Syntax & Region Access**: C-style block braces `{}` without mandatory semicolons, optional type annotations (`float`, `int`, `string`, `var`), dual comment styles (`//` and `#`), and C++ region-based access modifiers (`public:`, `private:`, `protected:`).
  - **Colon Class & Interface Inheritance**: C++ colon syntax without semicolons for inheritance and multiple interfaces (`class DragonBoss : EnemyScript, IDamageable, IPatroller`). A new `class` keyword implicitly concludes the preceding class scope.
  - **Unified `T_REGISTER_PROPERTY` Macro**: TScript uses the identical C++ `T_REGISTER_PROPERTY(float, speed, 300.0)` macro, inheriting property drawers, inspector controls, and future clamping bounds (`Min=`, `Max=`).
  - **Pre-Compiled Reflection Bridge**: Core macros (`TE_CORE_INFO`, `TE_CLIENT_LOG`, `TE_CORE_WARN`, `TE_CORE_ERROR`) and public/protected C++ object methods are pre-compiled into `TScriptInterpreter`'s C++ function binding table — guaranteeing zero external compiler dependencies (no MSVC, Clang, GCC, or Xcode required).
- [x] **Event-Driven Execution Engine**:
  - Event dispatchers: `on_ready()`, `on_update(float dt)`, `on_collision(other)`, `on_input(InputActionBinding[] bindings)`, `on_timer(string name)`, and `on_destroy()`.
  ```tscript
  // TScript class & event snippet
  class PlayerScript : TComponent, IDamageable
      T_REGISTER_PROPERTY(float, speed, 300.0)
      public:
          float health = 100.0
      private:
          int m_ammo = 10
      on_ready() {
          TE_CORE_INFO("Entity spawned: " + self.name)
          start_timer("Regen", 2.0, true)
      }
      on_update(float dt) {
          transform.position.x += speed * dt
      }
      on_input(InputActionBinding[] bindings) {
          for (var b : bindings) {
              if (b.action == "Jump" && b.triggered) {
                  transform.position.y += 150.0
              }
          }
      }
  ```

---

### 5. Intrinsic Multi-Script Component Architecture & Extensible Event Routing (Phase 3)
- [x] **Multi-Script Component Storage (`Engine/Include/GameFrameWork/TComponent.hpp`)**:
  - Upgrade `TComponent` to store `std::vector<TScriptInstance>`, allowing entities to hold multiple attached `.tscript` assets simultaneously.
  - Implement TScript Slots in `Engine/src/Editor/Panels/InspectorPanel.cpp` featuring Add, Remove, Enable Checkbox, and Open-In-Editor actions, serializing handles to `.tescene` YAML via `SceneSerializer.cpp`.
- [x] **`T_EVENT_VISIBLE` Macro System**:
  - Implement `T_EVENT_VISIBLE(EventType)` macro (`CollisionEvent`, `InputEvent`, `TimerEvent`, `AreaEvent`). Built-in colliders and custom user components declare `T_EVENT_VISIBLE(CollisionEvent)` to participate in event routing automatically.
  - Add an Event Visibility section in component inspector drawers showing active event hook badges (`on_ready ✓`, `on_update ✓`, `on_collision ✓`).
  ```cpp
  // Event visibility macro snippet
  class BoxColliderComponent : public CollisionComponent {
      T_EVENT_VISIBLE(CollisionEvent)
      // ...
  };
  ```

---

### 6. Shipped-Engine 2D Component Suite (Phase 4)
- [x] **Physics & Collision Suite (`Engine/Include/Core/Scene/` & `Collision/`)**:
  - **`RigidBody2DComponent`** (`RigidBody2DComponent.hpp`): Bi-directional transform sync between `Entity` and `PhysicsWorld` XPBD rigid bodies (`Static`, `Kinematic`, `Dynamic`, `Mass`, `GravityScale`, `LinearDamping`, `ApplyImpulse`).
  - **`CharacterBody2DComponent`** (`CharacterBody2DComponent.hpp`, extends `MovementComponentBase`): Kinematic character controller supporting `TopDown` and `SideScroller` modes with slope limits, step heights, and `move_and_slide()` physics resolution.
  - **`Area2DComponent`** (`Area2DComponent.hpp`, extends `CollisionComponent`): Trigger overlap zone firing `on_area_entered` and `on_area_exited` TScript events.
  - **`RayCast2DComponent`** (`RayCast2DComponent.hpp`): Per-frame ray testing exposing hit entities, hit normals, and intersection points to TScript.
  - **`CapsuleColliderComponent`** (`CapsuleColliderComponent.hpp`, extends `CollisionComponent`): Rounded-rectangle collision geometry for character bodies.
- [/] **Rendering Suite (`Engine/Include/Core/Scene/`)**:
  - **`AnimatedSpriteComponent`** (`AnimatedSpriteComponent.hpp`): Flipbook animation player with clip maps (`AnimationClip`), dynamic UV calculations, sequence tags, and Play/Pause/Stop API.
  - **`TilemapComponent` & `TileSetAsset`** (`TilemapComponent.hpp` & `TileSetAsset.hpp`): Batched grid layer renderer integrating with `Renderer2D` and auto-compiling static tile colliders into `PhysicsWorld`. Includes editor viewport paint tool.
  - **`GPUParticles2DComponent`** (`GPUParticles2DComponent.hpp`): Emitter with lifetime, velocity, direction, cone spread, color gradient, and gravity curve controls.
  - **`Line2DComponent`** (`Line2DComponent.hpp`): Polyline path renderer for ropes, laser beams, and debug paths.
- [x] **Audio & UI/HUD Suite (`Engine/Include/Core/Audio/` & `Scene/`)**:
  - **`AudioSource2DComponent` & `AudioListenerComponent`** (`AudioSource2DComponent.hpp` & `AudioListenerComponent.hpp`): Spatial 2D audio emitter and listener powered by an embedded MiniAudio backend (`miniaudio.h` via `AudioEngine.hpp`).
  - **`CanvasLayerComponent`** (`CanvasLayerComponent.hpp`): Fixed screen-space UI viewport layer unaffected by camera movement.
  - **`UIWidgetComponent`**, **`TextComponent`**, **`ButtonComponent`**, **`ProgressBarComponent`**, **`NinePatchRectComponent`**: Screen-space HUD widgets with click callbacks and visual state styles.
  ```tscript
  // Character movement TScript snippet
  on_update(float dt) {
      var vel = character.velocity
      vel.y += 980.0 * dt
      if (input_pressed("MoveRight")) { vel.x = 300.0 }
      if (character.is_on_floor() && input_triggered("Jump")) { vel.y = -500.0 }
      character.velocity = vel
      character.move_and_slide(dt)
  }
  ```

---

### 7. Core AI, State Trees & Navigation (Phase 5)
- [x] **Hierarchical State Trees (Engine Core, `Engine/Include/Core/AI/StateTree.hpp`)**:
  - **Hierarchical State Engine**: Implement `StateTree`, `StateNode`, `StateTransition`, and `StateTreeEvaluator` for data-oriented state execution (`Patrol`, `Chase`, `Attack`, `Flee`).
  - **Built-in Tasks & Conditions**: Implement `STMoveToPositionTask`, `STMoveToEntityTask`, `STPlayAnimationTask`, `STExecuteTScriptTask`, `STDistanceCondition`, `STHealthCondition`, `STHasTagCondition`, and `STTScriptCondition`.
  - **Visual Editor & Debugger (`Engine/src/Editor/StateTreeAssetEditor.cpp`)**: `.testatetree` YAML serializer and dockable node graph editor with real-time green/red state highlighting during PIE.
  ```yaml
  # .testatetree YAML snippet
  InitialState: Patrol
  States:
    - Name: Patrol
      TickTasks:
        - type: STMoveToPositionTask
      Transitions:
        - condition:
            type: STDistanceCondition
            target: Player
            max_distance: 250
          to: Chase
  ```
- [x] **Navigation & Pathfinding (Engine Core, `Engine/Include/Core/AI/NavigationMesh2D.hpp`)**:
  - **`NavigationMesh2D` & A\* Solver**: Grid/polygon navigation mesh generator with dynamic obstacle avoidance.
  - **`NavigationAgent2DComponent`** (`NavigationAgent2DComponent.hpp`): ECS component following A* waypoints with configurable stopping distance and automatic re-pathing timers.
  - **`TimerComponent`** (`TimerComponent.hpp`): Entity-attached timer manager firing `on_timer(name)` TScript events for single-shot or repeating timers.

---

### 8. Modular Optional Engine Plugins & AI/ML Extension Suite
- [x] **`GameplayTagPlugin` (Optional Plugin, `Engine/Plugins/GameplayTagPlugin/`)**:
  - Standalone opt-in plugin providing hierarchical string tags (`Character.Enemy.Boss.Dragon`), `GameplayTagContainer`, and `GameplayTagManager`.
  ```tscript
  // Gameplay tag query snippet
  on_collision(other) {
      if (other.has_tag("Character.Enemy")) { health -= 25.0 }
      if (other.has_tag("Status.Invincible")) { return }
  }
  ```
- [x] **`SpriteEditorPlugin`**: In-editor sprite raster and vector editing suite with pixel paint, vector editor, code editor submodes, and export pipeline.
- [x] **`DialogueRunnerPlugin`**: Node-based branching dialogue graph system with Ink/Yarn interpreters, localization tables, and quest integration.
- [x] **`SoundSynthesizerPlugin`**: Procedural procedural audio and waveform synthesis graph with DSP nodes, oscillators, envelopes, and sequencer.
- [x] **`PCGPlugin`**: 2D Procedural Content Generation toolset and node graph pipeline with point data, noise, structural generators, and ECS processors.
- [x] **`Skeletal2DPlugin`**: 2D bone hierarchy, inverse kinematics, skinning tables, timeline editor, and Spine JSON import.
- [x] **`ParticleFXPlugin`**: 2D particle simulation and effects runtime with node graph editor, emitters, spawn/update modules, and rendering.
- [x] **`MaterialSystemPlugin`**: Custom multi-pass shaders and material asset pipeline with visual shader graphs and uniform blocks.
- [x] **`RichTextPlugin`**: Tag-based rich formatting, dynamic text effects, layout engine, and animated text styling.
- [x] **`TextToAudioPlugin`**: Offline text-to-speech and audio cue synthesis with Windows SAPI backend and TTS asset editor.
- [x] **`IKPlugin`**: 2-bone, CCD, and FABRIK analytical 2D/3D inverse kinematics solver plugin with foot grounding constraints.
- [ ] **`NeuralAIPlugin`**: Lightweight neural network inference node for state choice evaluation offloaded via `SUBMIT_AI()`.
- [ ] **`MotionMatchingPlugin`**: 2D feature-vector nearest-neighbor search for fluid 2D sprite animation blending.
- [ ] **`MLLevelGenPlugin`**: ONNX-based procedural level and room layout generation plugin.
- [ ] **`AdaptiveDifficultyPlugin`**: Player performance metrics tracker dynamically scaling difficulty multipliers.
- [ ] **`TextureUpscalerPlugin`**: ESRGAN offline 4x sprite texture upscaler plugin.
- [ ] **`LLMDialoguePlugin`**: Async local LLM integration for dynamic NPC dialogue generation.
- [ ] **`Lighting2DPlugin`**: 2D dynamic shadows, ambient lights, and normal mapping.
- [ ] **`CheatDetectionPlugin`**: Deterministic state integrity and speedhack verification.
- [ ] **`AnalyticsHeatmapPlugin`**: Player telemetry and spatial heatmap visualization.
- [ ] **`PhysicsSurrogatePlugin`**: Specialized 2D physics constraints and joints.
- [ ] **`AdaptiveMusicPlugin`**: Multi-track dynamic stem mixing and transitions.

---

### 9. Dedicated Git Editor Layer & Local Version Control System (Phase 6)
- [ ] **Asynchronous Git CLI Bridge (`Engine/Include/Core/VCS/GitBridge.hpp` & `GitBridge.cpp`)**:
  - **Subprocess Pipeline**: High-performance, non-blocking asynchronous wrapper around `git` CLI operations using `PlatformUtils::LaunchProcess()` and pipe streaming.
  - **Operations API**: Real-time status query (staged, unstaged, untracked, merge-conflicted), commit history graph parsing (hashes, parents, authors, dates, branches), unified text & asset diff generation, branch creation/checkout/merge, and push/pull/fetch/stash controls.
- [ ] **Lightweight Local History & Shadow Checkpoints (`Engine/Include/Core/VCS/LocalHistoryManager.hpp` & `LocalHistoryManager.cpp`)**:
  - **Shadow Snapshot Storage**: Save timestamped text diffs/snapshots into `<ProjectDir>/.te/history/` automatically on manual scene save, autosave timer, before PIE playtest, and before Git operations.
  - **Rolling Retention Pruning**: Automatic disk cleanup maintaining a rolling window (last $N$ snapshots or age threshold) ensuring zero memory/disk bloat.
  - **Standalone Engine-Agnostic API**: Clean C++ library (`TELocalHistory`) callable by CLI utilities and external tools without UI dependencies.
- [ ] **Dedicated Git Editor Workspace Layer (`Engine/Include/Layers/GitEditorLayer.hpp` & `GitEditorLayer.cpp`)**:
  - **Visual Commit DAG Graph**: Interactive node graph rendering branch lanes, merge points, tags, and commit nodes with author badges.
  - **Side-by-Side Unified Diff Viewer**: Dual-column syntax-highlighted diff viewer for `.tescene`, `.tscript`, `.teproj`, and `.temat` with selective line/hunk staging.
  - **3-Way Visual Merge Conflict Resolver**: Conflict editor showing *Current (Ours)*, *Incoming (Theirs)*, and *Merged Result* with single-click resolution buttons.
  - **Stash & Branch Manager**: Interactive sidebar for branch switching, stash stack management, and remote sync.
  - **Unified Timeline**: Chronological view merging official Git commits with local `.te/history` shadow snapshots.
- [ ] **Dockable Version Control Panel (`Engine/Include/Editor/Panels/VersionControlPanel.hpp` & `VersionControlPanel.cpp`)**:
  - Lightweight dockable panel within standard `EditorLayer` for quick staging, commit authoring, and one-click workspace switching to `GitEditorLayer`.

---

### 10. TimeEditor Application Infrastructure (Phase 7)
- [ ] **Crash Reporter & MiniDump Generator (`TimeEditor/src/CrashHandler.hpp` & `CrashHandler.cpp`)**:
  - Global unhandled exception filter (`SetUnhandledExceptionFilter` on Windows) generating timestamped `.dmp` and `.log` files in `Saved/Crashes/`.
  - Emergency auto-save hook writing active scene state before process exit to prevent lost work.
- [ ] **Single-Instance Guard & Named Pipe IPC (`TimeEditor/src/SingleInstanceGuard.hpp` & `SingleInstanceGuard.cpp`)**:
  - Win32 Named Pipe & Mutex system ensuring double-clicking `.teproj` / `.tescene` files forwards an open request to the existing editor window rather than launching duplicate instances.
- [ ] **Headless Automation & CLI Asset Cooker (`TimeEditor/src/TimeEditorApplication.cpp`)**:
  - Command-line arguments for automated CI/CD: `--cook` (asset packaging & stripping), `--run-tests` (headless test suite execution), and `--export-project`.
- [ ] **Project Starter Templates (`TimeEditor/Templates/`)**:
  - Starter templates selectable in `ProjectHubLayer`: `Templates/2D_Platformer/`, `Templates/TopDown_RPG/`, and `Templates/Blank/`.
- [ ] **Editor Static Resources & Themes (`TimeEditor/Resources/`)**:
  - High-DPI UI icon font packs, viewport grid shaders, missing texture fallbacks, and customizable color themes (`DarkTheme.ini`, `LightTheme.ini`).

---

### 11. Core Editor Tooling Suite & Level Design (Phase 8)
- [ ] **Global Undo / Redo Command Stack (`Engine/Include/Editor/EditorCommandManager.hpp` & `EditorCommandManager.cpp`)**:
  - Unified command pattern (`IEditorCommand`) recording entity transforms, property edits, hierarchy changes, and component additions with `Ctrl+Z` / `Ctrl+Y` and an interactive History Panel.
- [ ] **2D Tilemap Painter Panel (`Engine/Include/Editor/Panels/TilemapPainterPanel.hpp` & `TilemapPainterPanel.cpp`)**:
  - Visual tile palette picker, brush / eraser / bucket fill tools, multi-layer management, grid snapping, and automatic static collision generation in `PhysicsWorld`.
- [ ] **Performance Profiler & Debug Diagnostics Panel (`Engine/Include/Editor/Panels/ProfilerPanel.hpp` & `ProfilerPanel.cpp`)**:
  - Real-time CPU frame time vs. GPU frame time timeline graphs, batched draw call metrics, quad/vertex counters, texture VRAM tracking, and toggleable XPBD physics collision debug overlays.
- [ ] **Project Packaging & Release Export Panel (`Engine/Include/Editor/Panels/ProjectPackagingPanel.hpp` & `ProjectPackagingPanel.cpp`)**:
  - Standalone release packaging wizard for Windows x64 and Linux with automated asset stripping, script AST pre-caching, and standalone executable bundling.

---

## 🔴 Long-Term Infrastructure (Missing Systems)

### 1. Editor UI Logging & Console
- [x] **Logger Filter Control**:
  - Expose customizable logger controls (filter by severity/category) inside the **Editor Settings** panel.
- [x] **Output Log Window**:
  - Integrate a live GUI logger window displaying core and client logs natively.
- [x] **Built-in Editor Terminal**:
  - Integrate a command-line console interface directly inside the workspace window.

### 2. Audio Engine
- [x] **Embedded MiniAudio Integration**:
  - Integrate embedded MiniAudio (`miniaudio.h`) backend in `Engine/Include/Core/Audio/AudioEngine.hpp`.
  - Create `AudioSource2DComponent` and `AudioListenerComponent` ECS systems.
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

> **Current State**: TimeEngine is fully tested and supported for development on **Windows** and **macOS** via Premake5 project generators and build script suites (`Scripts/Windows/` and `Scripts/Mac/`). Linux scripts exist (`Scripts/Linux/`) but are currently untested.

#### Platform Status

| Platform | Compiler | IDE/Build | Status |
|---|---|---|---|
| Windows (MSVC/Clang) | MSVC / Clang | Visual Studio 2022 `.sln` | ✅ Fully Tested / Supported |
| macOS (AppleClang) | AppleClang | Premake → `xcode4` / Makefile | ✅ Fully Tested / Supported |
| Linux (GCC/Clang) | GCC or Clang | Premake → `gmake2` | ⚠️ Scripts exist, untested |
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
