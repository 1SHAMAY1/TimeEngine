# TimeEngine Development Roadmap

This document outlines the current state and planned features of TimeEngine, structured into coordinated Milestones, Issues, Sub-issues, and Subsystem Tags.

---

## 🟢 Completed Milestones (Production-Ready)

### [Milestone: "Multi-API Renderer & Windowing Backend" | Section: Completed]
> **Description**: High-performance multi-backend graphics rendering pipeline (OpenGL 4.5, Vulkan, OpenGLES, DirectX 11) with strict vendor encapsulation.

#### [Issue: "Batched 2D Renderer Engine" | Tags: Renderer, 2D | Status: completed]
- **Description**: Core batched rendering pipeline (`Renderer2D`, `RenderBatcher`) supporting thousands of textured quads and geometry in minimal draw calls.
- **Sub-tasks**:
  - [x] Subissue: Multi-API backend abstraction (OpenGL 4.5 DSA, Vulkan 1.3, DX11, GLES 3.0)
  - [x] Subissue: Automatic component drawers (`BoxComponent`, `CircleComponent`, `TriangleComponent`)
  - [x] Subissue: Strict vendor isolation wrappers (`TimeGUI`, `IWindow`, `MathUtils`)

---

### [Milestone: "Serialization & Core File Management" | Section: Completed]
> **Description**: Full YAML-based scene and project serialization system with dynamic asset registration and caching.

#### [Issue: "YAML Scene & Project Serializer" | Tags: Scene, Project, Asset | Status: completed]
- **Description**: Robust YAML deserialization and serialization preserving entity components, properties, and project configurations.
- **Sub-tasks**:
  - [x] Subissue: Full YAML-based scene state serialization (`SceneSerializer`)
  - [x] Subissue: Dynamic asset prototype registry and directory asset cache (`AssetManager`)

---

### [Milestone: "Editor & Automation Systems" | Section: Completed]
> **Description**: Runtime plugin discovery and remote AI model context protocol (MCP) JSON-RPC over SSE.

#### [Issue: "Runtime Plugins & MCP Server" | Tags: Editor, Plugin, MCP, AI Agents | Status: completed]
- **Description**: Modular `.teplugin` runtime loading interface with an embedded SSE automation server on port 3000.
- **Sub-tasks**:
  - [x] Subissue: Runtime plugin manager GUI (`Edit -> Plugins`) with dynamic DLL load/unload
  - [x] Subissue: Remote MCP JSON-RPC over SSE server on port 3000 with screen capture and tools

---

## 🟡 Short-Term Goals (Next Steps)

### [Milestone: "ECS Component & Gameplay Framework" | Section: Short-Term]
> **Description**: Expands TimeEngine core ECS components, 2D player controllers, flipbook animations, and level design tools.

#### [Issue: "Parallax Scrolling Component" | Tags: ECS, Renderer, 2D | Status: in-progress]
- **Description**: Integrate `ParallaxComponent` with `Renderer2D` rendering passes and expose depth speed controls in `TEPropertyDrawer`.
- **Sub-tasks**:
  - [/] Subissue: Integrate `ParallaxComponent` into `Renderer2D` rendering pass
  - [ ] Subissue: Expose depth speed inspectors inside `TEPropertyDrawer`

#### [Issue: "Animated Sprite Flipbook Renderer" | Tags: ECS, Renderer, 2D | Status: todo]
- **Description**: Implement full flipbook render loop integration into the batched renderer.
- **Sub-tasks**:
  - [ ] Subissue: Integrate flipbook animation player with clip maps and sequence tags into `Renderer2D`
  - [ ] Subissue: Texture atlas dynamic UV coordinate calculations

#### [Issue: "2D Gameplay Controller & Logic Framework" | Tags: ECS, GameFrameWork | Status: todo]
- **Description**: Complete the player base controller stubs and implement level trigger volumes and game loop state bindings.
- **Sub-tasks**:
  - [ ] Subissue: Complete `PlayerControllerBase.hpp` and `2DPlayerController.hpp`
  - [ ] Subissue: Implement gameplay logic systems, trigger volumes, and game loop managers

#### [Issue: "Tilemap & Level Design Core System" | Tags: Editor, 2D, Physics | Status: todo]
- **Description**: Tilemap painting system with custom brushes and compound static collider compilation.
- **Sub-tasks**:
  - [ ] Subissue: Auto-tiling brush with custom rules and layer support
  - [ ] Subissue: Dynamic physics collider compilation to eliminate collision snagging

#### [Issue: "Scripting Sandboxing & Determinism" | Tags: Core, Scripting | Status: todo]
- **Description**: Lightweight deterministic scripting bindings with instant state serialization for time rollbacks.
- **Sub-tasks**:
  - [ ] Subissue: Fast scripting language integration with state-safe variable sandboxes
  - [ ] Subissue: Instant state serialize/deserialize routines for frame rollbacks

#### [Issue: "Visual Behavior Tree & AI Node Graph" | Tags: AI Agents, Editor, ECS | Status: todo]
- **Description**: Visual node graph for Selector, Sequence, Decorator, and Action nodes with real-time execution debugger.
- **Sub-tasks**:
  - [ ] Subissue: Interactive node graph linking selectors, sequences, decorators, and actions
  - [ ] Subissue: Real-time simulation debugger with active execution path highlighting
  - [ ] Subissue: Automatic task registrator scanning gameplay code

#### [Issue: "VFX & Particle Designer Studio" | Tags: Renderer, Editor, 2D | Status: todo]
- **Description**: Particle emitter node graph with visual curves for lifetimes, velocity, color gradients, and real-time playback preview.
- **Sub-tasks**:
  - [ ] Subissue: Emitter node graph for particle lifetimes, velocity curves, and gravity modifiers
  - [ ] Subissue: Viewport VFX playback panel for real-time previewing without starting game loop

---

### [Milestone: "Physics Engine Integration (Velox)" | Section: Short-Term]
> **Description**: Extended collision geometry support and real-time property syncing with the Velox XPBD physics solver.

#### [Issue: "Extended 2D Collision Geometry" | Tags: Physics, Collision, 2D | Status: todo]
- **Description**: Expose triangle and polygon colliders in `PhysicsWorld::AddBody` and synchronize velocities with editor properties.
- **Sub-tasks**:
  - [ ] Subissue: Expose Triangle and Polygon colliders in `PhysicsWorld::AddBody`
  - [ ] Subissue: Real-time feedback loop when setting entity velocity in property panels

---

### [Milestone: "Tooling, PCG & Content Automation" | Section: Short-Term]
> **Description**: In-engine procedural generation, translation tooling, sprite sheet packagers, and developer automation.

#### [Issue: "Procedural Content Generation (PCG) Plugin" | Tags: Plugin, PCG, 2D | Status: todo]
- **Description**: Node-based layout generation pipeline using `PCGPointData`, noise nodes, structural maze creators, and 12 ECS processors.
- **Sub-tasks**:
  - [ ] Subissue: Point data generators (Grid, Random/Scatter, Poisson Disk, Path/Line)
  - [ ] Subissue: Noise and terrain nodes (Perlin/Simplex/Worley, Elevation, Slope)
  - [ ] Subissue: Structural creators (Wilson's Maze, Cellular Automata, BSP rooms, Drunkard's Walk, WFC solver)
  - [ ] Subissue: 12 specialized ECS-compatible processor nodes (Voronoi, Delaunay, Relaxer, Raycast, etc.)

#### [Issue: "Localization & Translation Tooling Plugin" | Tags: Plugin, Utils, Docs | Status: todo]
- **Description**: Spreadsheet-style translation dictionary grids with dynamic length visualizers and in-editor locale switchers.
- **Sub-tasks**:
  - [ ] Subissue: Dictionary grid UI with dynamic length visualizer and live locale previewing
  - [ ] Subissue: CSV/JSON compiler scripts for `.telocale` binary format
  - [ ] Subissue: `LocalizedTextComponent` dynamic updating and locale font override system

#### [Issue: "Sprite Sheet & Atlas Packager Plugin" | Tags: Plugin, Editor, 2D | Status: todo]
- **Description**: MaxRects bin packing layout engine with border trimming, pixel extrusion, and `.teatlas` asset exporters.
- **Sub-tasks**:
  - [ ] Subissue: MaxRects bin packing algorithm with Power-of-Two constraints
  - [ ] Subissue: Transparent border trimming and pixel extrusion dilation settings
  - [ ] Subissue: Coordinate mapping JSON exporter and `AnimatedSpriteComponent` loader

#### [Issue: "Cross-Platform Automation Suite" | Tags: Automation, CI/CD, Platform | Status: in-progress]
- **Description**: Symmetric script suites for Windows, Linux, and macOS for cleaning, generating workspaces, and building.
- **Sub-tasks**:
  - [x] Subissue: Windows MSVC/MinGW/Clang workspace generation and build scripts
  - [/] Subissue: Linux and macOS validation and test automation
  - [ ] Subissue: Unified cross-platform CLI tool wrappers (`Scripts/MCP_Tools.sh`)

---

### [Milestone: "Play-In-Editor (PIE) & Standalone Runtime" | Section: Short-Term]
> **Description**: Deep-copy scene simulation states (Play, Pause, Step, Resume, Stop) and standalone game launcher processes.

#### [Issue: "Play-In-Editor (PIE) State Machine" | Tags: Editor, Scene, Core | Status: todo]
- **Description**: Deep-copy `m_EditorScene` into `m_RuntimeScene` when hitting Play, ensuring editor scenes remain pristine.
- **Sub-tasks**:
  - [ ] Subissue: In-memory scene copying (`Scene::Copy()`) for runtime isolation
  - [ ] Subissue: 3-state toolbar UI (Play, Pause, Resume, Stop) with step debugging
  - [ ] Subissue: Standalone process launcher (`--standalone --project=<path> --scene=<path>`) via `GameLayer`

---

### [Milestone: "TScript Embedded Language Engine" | Section: Short-Term]
> **Description**: Embedded C/Python hybrid scripting language requiring zero external compilers, with AST caching and reflection bridges.

#### [Issue: "TScript AST Caching & Event Engine" | Tags: Core, Scripting | Status: completed]
- **Description**: Pratt parser tokenizing `.tscript` source into cached AST programs with pre-compiled C++ function binding tables.
- **Sub-tasks**:
  - [x] Subissue: Single-pass lexer and Pratt parser building typed AST (`TScriptAST.hpp`)
  - [x] Subissue: AST caching inside `TScriptAsset` to eliminate runtime string re-parsing
  - [x] Subissue: Hybrid C/Python syntax with access modifiers and inheritance
  - [x] Subissue: Event dispatchers (`on_ready`, `on_update`, `on_collision`, `on_input`, `on_timer`, `on_destroy`)

---

### [Milestone: "Shipped 2D Component Suite & State Trees" | Section: Short-Term]
> **Description**: Production 2D character bodies, raycasts, audio emitters, screen-space UI widgets, and hierarchical AI state trees.

#### [Issue: "2D Character Bodies & Physics Components" | Tags: ECS, Physics, 2D | Status: completed]
- **Description**: Kinematic character controller with `move_and_slide()` and overlap trigger zones.
- **Sub-tasks**:
  - [x] Subissue: `CharacterBody2DComponent` with slope limits and `move_and_slide()` resolution
  - [x] Subissue: `Area2DComponent` trigger zones and `RayCast2DComponent` hit tests
  - [x] Subissue: `AudioSource2DComponent` and `AudioListenerComponent` spatial audio

#### [Issue: "Hierarchical AI State Trees & Navigation" | Tags: AI Agents, ECS, Core | Status: completed]
- **Description**: Hierarchical state trees with transition conditions and 2D A* navigation mesh pathfinding.
- **Sub-tasks**:
  - [x] Subissue: `StateTree`, `StateNode`, and `StateTreeEvaluator` with task conditions
  - [x] Subissue: Visual editor and `.testatetree` YAML serializer with real-time PIE debugger
  - [x] Subissue: `NavigationMesh2D` polygon mesh with `NavigationAgent2DComponent` A* solver

---

### [Milestone: "Modular Plugin Ecosystem" | Section: Short-Term]
> **Description**: Dynamic optional extension plugins for AI, procedural audio, dialogue, skeletal animation, and lighting.

#### [Issue: "Completed Foundation Plugins" | Tags: Plugin, Editor, Audio | Status: completed]
- **Description**: Shipped plugin collection providing specialized capabilities.
- **Sub-tasks**:
  - [x] Subissue: `GameplayTagPlugin` hierarchical string tags and container queries
  - [x] Subissue: `SpriteEditorPlugin` pixel paint, vector editor, and code editor submodes
  - [x] Subissue: `DialogueRunnerPlugin` node-based branching dialogue with Ink/Yarn
  - [x] Subissue: `SoundSynthesizerPlugin` procedural waveform synthesizer graph
  - [x] Subissue: `PCGPlugin` 2D procedural generation pipeline
  - [x] Subissue: `Skeletal2DPlugin` bone hierarchies and Spine JSON import
  - [x] Subissue: `ParticleFXPlugin` node graph particle simulation
  - [x] Subissue: `MaterialSystemPlugin` multi-pass shader graphs
  - [x] Subissue: `RichTextPlugin` dynamic text markup, font metrics, and animators
  - [x] Subissue: `TextToAudioPlugin` offline SAPI text-to-speech synthesis
  - [x] Subissue: `IKPlugin` 2-bone, CCD, and FABRIK inverse kinematics

#### [Issue: "Planned Extension Plugins" | Tags: Plugin, AI Agents, 2D | Status: todo]
- **Description**: High-level advanced plugins under active design.
- **Sub-tasks**:
  - [ ] Subissue: `NeuralAIPlugin` inference offloaded via `SUBMIT_AI()`
  - [ ] Subissue: `MotionMatchingPlugin` nearest-neighbor search for 2D sprite blending
  - [ ] Subissue: `MLLevelGenPlugin` ONNX procedural room layout generator
  - [ ] Subissue: `AdaptiveDifficultyPlugin` player telemetry scaler
  - [ ] Subissue: `TextureUpscalerPlugin` ESRGAN 4x sprite upscaler
  - [ ] Subissue: `LLMDialoguePlugin` async local LLM NPC dialogue
  - [ ] Subissue: `Lighting2DPlugin` 2D normal mapping and dynamic shadows
  - [ ] Subissue: `CheatDetectionPlugin` deterministic state integrity checker

---

### [Milestone: "In-Engine Version Control & Git Workspace" | Section: Short-Term]
> **Description**: Asynchronous non-blocking Git CLI bridge, rolling shadow local history, and dedicated visual Git workspace layer.

#### [Issue: "Git Subprocess Bridge & Local History" | Tags: Core, VCS, Automation | Status: todo]
- **Description**: Subprocess pipe streaming for Git operations and automated `.te/history/` shadow text snapshots.
- **Sub-tasks**:
  - [ ] Subissue: Non-blocking asynchronous `GitBridge` wrapper using process pipes
  - [ ] Subissue: Local history manager saving timestamped snapshots with rolling retention pruning

#### [Issue: "Dedicated Git Editor Workspace Layer" | Tags: Editor, VCS, Layers | Status: todo]
- **Description**: Interactive commit DAG graph, dual-column diff viewer, 3-way visual merge conflict resolver, and branch manager.
- **Sub-tasks**:
  - [ ] Subissue: Interactive node graph rendering branch lanes, merge points, and commit nodes
  - [ ] Subissue: Side-by-side syntax-highlighted diff viewer with hunk staging
  - [ ] Subissue: 3-way visual merge conflict resolver
  - [ ] Subissue: Dockable Version Control panel for quick staging and committing

---

### [Milestone: "Editor Infrastructure, Tools & Release Packaging" | Section: Short-Term]
> **Description**: Crash reporter, single-instance IPC, global undo/redo command stack, 2D tilemap painter, and release packager.

#### [Issue: "Crash Handling & IPC Automation" | Tags: Editor, Platform, Automation | Status: todo]
- **Description**: Global unhandled exception filter generating `.dmp` files and Win32 Named Pipe single-instance guard.
- **Sub-tasks**:
  - [ ] Subissue: Global crash reporter generating timestamped `.dmp` files with emergency scene auto-save
  - [ ] Subissue: Named Pipe IPC ensuring single-instance execution on `.teproj` open
  - [ ] Subissue: Headless automation CLI arguments (`--cook`, `--run-tests`, `--export-project`)

#### [Issue: "Editor Tooling & Project Packaging Wizard" | Tags: Editor, Platform, Build | Status: todo]
- **Description**: Global Undo/Redo command stack (`IEditorCommand`), real-time CPU/GPU profiler graphs, and standalone release exporter.
- **Sub-tasks**:
  - [ ] Subissue: Global Undo/Redo command pattern (`IEditorCommand`) with History Panel
  - [ ] Subissue: 2D tilemap painter panel with palette picker and auto-collision generator
  - [ ] Subissue: Profiler panel with CPU/GPU timeline graphs, draw call counters, and VRAM metrics
  - [ ] Subissue: Release packaging wizard with asset stripping, AST pre-caching, and binary bundling

---

## 🔴 Long-Term Infrastructure (Missing Systems)

### [Milestone: "Deterministic Time Manipulation & Netcode" | Section: Long-Term]
> **Description**: Fixed-timestep simulation decoupling, component snapshot buffers, time dilation audio pitch link, and rollback netcode.

#### [Issue: "Time Rewind & State Rollback Engine" | Tags: Core, Physics, Time | Status: todo]
- **Description**: Separate rendering frame loops from fixed physics updates and implement delta-compressed snapshot rewind buffers.
- **Sub-tasks**:
  - [ ] Subissue: Decouple variable rendering frame loops from fixed-timestep physics updates
  - [ ] Subissue: Component snapshot buffers and delta-compression for saving/restoring historical entity positions
  - [ ] Subissue: Frame-rewinding playback API controls
  - [ ] Subissue: Audio pitch-to-time dilation real-time modulation

#### [Issue: "Deterministic Multiplayer Rollback Netcode" | Tags: Core, Time, Network | Status: todo]
- **Description**: Client-side prediction and rollback network framework utilizing snapshot rollback capabilities.
- **Sub-tasks**:
  - [ ] Subissue: Rollback sync network framework using engine state rewinds
  - [ ] Subissue: Conflict frame resolution and instant fast-forward resimulation

---

### [Milestone: "Universal Cross-Platform Toolchains" | Section: Long-Term]
> **Description**: Full compilation and runtime support across Linux, macOS, WebAssembly, and mobile platforms.

#### [Issue: "Linux & macOS Platform Layers" | Tags: Platform, Linux, MAC, Build | Status: in-progress]
- **Description**: Premake compiler guards, `dlopen` plugin loader on non-Windows, and Cocoa/X11 platform implementations.
- **Sub-tasks**:
  - [ ] Subissue: Guard MSVC/DirectX flags in `Premake5.lua` and add platform-conditional OpenGL/X11/Cocoa links
  - [ ] Subissue: Refactor `PluginManager.cpp` to use `dlopen`/`dlclose` on Linux/macOS
  - [ ] Subissue: Complete `Engine/src/Platform/Linux/` and `Engine/src/Platform/Mac/` implementations
  - [ ] Subissue: Resolve macOS MoltenVK / Metal backend direction

#### [Issue: "Cross-Platform CI/CD Build Farm" | Tags: CI/CD, Build, Platform | Status: in-progress]
- **Description**: Independent matrix workflows for Windows (MSVC/Clang/MinGW), Linux (GCC/Clang), macOS (Intel/ARM), and WASM.
- **Sub-tasks**:
  - [x] Subissue: Windows MSVC x64 build matrix in CI
  - [ ] Subissue: Windows Clang-cl and MinGW-w64 CI matrix jobs
  - [ ] Subissue: Linux GCC and Clang Ubuntu CI matrix jobs
  - [ ] Subissue: macOS AppleClang Intel (macOS-13) and Apple Silicon ARM (macOS-14) CI matrix jobs
  - [ ] Subissue: Emscripten/WASM WebGL2 demo compiler pipeline
