# TimeEngine — LLM Reference

## Identity
- **Type**: Custom C++ game engine (2D primary, 3D secondary)
- **Core differentiator**: Deterministic simulation + Time Manipulation (rewind, slow-motion, state-branching)
- **Status**: Active development. Not all systems are complete. Do not assume a feature exists unless listed below.

## Core Architecture
- **Language**: C++20
- **Graphics API**: OpenGL 4.5+, Vulkan, DirectX 11, OpenGL ES
- **Architecture**: Entity-Component System (ECS)
- **Build System**: Premake5 / MSBuild
- **UI**: TimeGUI (Strict ImGui Abstraction Wrapper)

## Key APIs & Systems
- **Renderer**: `Renderer2D` and `Renderer3D` (optimized batching for quads/sprites). Supports Vulkan, OpenGL ES, DirectX 11, and OpenGL core profile.
- **Physics**: `PhysicsWorld` (Velox Physics Engine) — rigid body simulation and collision resolution via XPBD solver.
- **Inbuilt 2D Sprite Editor & IDE**: Data-driven procedural scripting with recursive expression evaluation.
- **Scene System**: `Scene` class manages entities and components via ECS.
- **Serialization**: Scene and Project serialization (YAML).
- **Events**: Event systems for windowing, user input, and scene lifecycles.
- **Input**: Action-based input mapping.
- **Plugins Settings Panel**: Built-in GUI panel (toggled via `Edit -> Plugins`) showing discovered plugins and enabling runtime load/unload DLL manipulation.
- **MCP Automation & SSE Server**: Built-in server providing remote programmatic automation:
  - Port: `3000` (HTTP and SSE streams `/message`).
  - Active Tools:
    - *Metadata & Modes*: `get_engine_info`, `get_scene_hierarchy`, `get_editor_modes`, `set_editor_mode`.
    - *Entities & ECS*: `create_entity`, `destroy_entity`, `add_component`, `set_entity_properties`.
    - *Viewport Feedback*: `get_viewport_screenshot` (saves `temp_viewport_capture.png` locally).
    - *Callback Deletion*: `delete_screenshot` (triggered explicitly by AI client to safely erase the temp screenshot file post-processing, avoiding race condition loss).
    - *Simulated Input*: `send_editor_input` (simulates keys/clicks).
    - *File I/O*: `create_directory`, `delete_file_or_directory`.
  - Client shell wrapper script: `Scripts/MCP_Tools.sh`. Uses Python parsing script `Scripts/parse_json.py` to compile nested properties payload arguments safely on Windows.
- **Ambient Light**: `AmbientLightComponent` exists (`Engine/Include/Core/Scene/AmbientLightComponent.hpp`) and supports gradient/multi-color ambient lighting.
- **Console & Terminal Panel**: Toggled via `Window -> Console & Terminal` and docked at the bottom:
  - Output Log Tab: Shows core and client log history with real-time level/category filters and search capabilities.
  - Terminal Tab: Offers a CLI shell executing in the opened project's root folder (`Project::GetProjectDirectory()`), allowing engine-level commands (`help`, `fps`, `list_entities`, `create_entity`, `destroy_entity`) and native OS system commands.
- **Advanced Performance Profiler**: Real-time profiler dashboard toggled via the Viewport menu:
  - Timing Hooks: Tracks `GameTime` (Editor updates), `RenderTime` (batcher flush steps), `PhysicsTime` (simulation steps), and `UITime` (UI drawing).
  - Memory & Stack Profiling: Tracks heap/stack allocations. Supports scoped function stack allocation tracking using RAII `StackProfileScope` and class allocations:
    - Record allocation (e.g. in constructor): `ProfilingLayer::TrackClassAllocation("MyNewClass", sizeof(*this));`
    - Record deallocation (e.g. in destructor): `ProfilingLayer::TrackClassDeallocation("MyNewClass", sizeof(*this));`

## Development Patterns
- Use `TE_CORE_LOG` for engine-side logging.
- Use `TE_CLIENT_LOG` for sandbox/game logging.
- Prefer `Ref<T>` (smart pointers) for resource management.
- All files should include `#pragma once`.
- Components live under `Engine/Include/Core/Scene/` and register via macros (e.g. `T_REGISTER_COMPONENT`).

## Setup
1. Run the workspace generation script in `Scripts/` (e.g. `Scripts/Windows/GenerateProjectFiles.bat` for Windows).
2. Build with MSBuild or Visual Studio.
3. Launch TimeEditor to access the Project Hub.

## NOT yet implemented (do not hallucinate these)
- Time Manipulation runtime (rewind, snapshots, branching) — design phase only
- 3D physics
- Audio system
- Networking
- Cross-platform support (Windows only)

| Pattern | Rule |
|---|---|
| Logging (engine) | `TE_CORE_LOG` |
| Logging (game/sandbox) | `TE_CLIENT_LOG` |
| Resource ownership | `Ref<T>` (shared_ptr wrapper) |
| Headers | `#pragma once` always |
| Naming | PascalCase classes, camelCase variables, SCREAMING_SNAKE_CASE macros |
| Formatting | `.clang-format` enforced, 4-space indent |

## Vendor Wrapper Mapping
All third-party libraries have strict wrappers that isolate raw vendor classes, headers, and APIs. Direct calls to vendor headers in engine code are prohibited.

- **ImGui (UI/Editor Layouts)**:
  - Wrapper Headers: [TimeGUI.hpp](file:///E:/TimeEngine/Engine/Include/Utils/TimeGUI.hpp)
  - Wrapper Sources: [TimeGUI.cpp](file:///E:/TimeEngine/Engine/src/Utils/TimeGUI.cpp)
  - Lifecycle Layer: [TimeGUILayer.cpp](file:///E:/TimeEngine/Engine/src/Core/Layers/TimeGUILayer.cpp)
- **OpenGL/Glad/OpenGLES/Vulkan/DirectX11 (Graphics/Renderer)**:
  - Batched renderer: [Renderer2D.hpp](file:///E:/TimeEngine/Engine/Include/Renderer/Renderer2D.hpp) / [Renderer2D.cpp](file:///E:/TimeEngine/Engine/src/Renderer/Renderer2D.cpp)
  - Generic Renderer API: [RendererAPI.hpp](file:///E:/TimeEngine/Engine/Include/Renderer/RendererAPI.hpp)
  - Shaders: [Shader.hpp](file:///E:/TimeEngine/Engine/Include/Renderer/Shader.hpp)
  - Textures: [Texture.hpp](file:///E:/TimeEngine/Engine/Include/Renderer/Texture.hpp)
  - Colors: [TEColor.hpp](file:///E:/TimeEngine/Engine/Include/Renderer/TEColor.hpp) / [TEColor.cpp](file:///E:/TimeEngine/Engine/src/Renderer/TEColor.cpp)
- **Velox (Physics Engine)**:
  - Abstraction: [PhysicsWorld.hpp](file:///E:/TimeEngine/Engine/Include/Core/Physics/PhysicsWorld.hpp) / [PhysicsWorld.cpp](file:///E:/TimeEngine/Engine/src/Core/Physics/PhysicsWorld.cpp)
- **GLFW (Windowing & OS Input Integration)**:
  - OS abstraction interface: [IWindow.hpp](file:///E:/TimeEngine/Engine/Include/Core/IWindow.hpp)
  - Windows-specific implementation: [WindowsWindow.cpp](file:///E:/TimeEngine/Engine/src/Window/WindowsWindow.cpp)
- **Customizable Logger (Logging Backend)**:
  - System logging interface: [Log.h](file:///E:/TimeEngine/Engine/src/Core/Log.h)
- **GLM (Mathematics / Vectors / Matrices)**:
  - Math & Vectors wrapper: [MathUtils.hpp](file:///E:/TimeEngine/Engine/Include/Utils/MathUtils.hpp)
- **stb_image / Asset Importers (Image & Model Loaders)**:
  - Asset Manager / Importers: [AssetManager.hpp](file:///E:/TimeEngine/Engine/Include/Core/Asset/AssetManager.hpp) / [AssetManager.cpp](file:///E:/TimeEngine/Engine/src/Core/Asset/AssetManager.cpp)

## File Layout (partial — extend as codebase grows)
```
Engine/
  Include/
    Core/Scene/           # ECS components live here (e.g. AmbientLightComponent.hpp)
    Renderer/             # Renderer API and backend headers (DirectX11, Vulkan, OpenGLES, OpenGL)
    Layers/               # EditorLayer.hpp, etc.
  src/
    Core/Scene/           # Scene.cpp, SceneSerializer.cpp, EntityManager.cpp
    Renderer/             # Renderer2D.cpp, and backend API source files
    Core/Layers/          # EditorLayer.cpp, etc.
Scripts/
  Windows/
    GenerateProjectFiles.bat  # Run first on Windows — generates .sln via Premake5
  Linux/
    GenerateProjectFiles.sh   # Run first on Linux — generates Makefiles via Premake5
  Mac/
    GenerateProjectFiles.sh   # Run first on macOS — generates Xcode project via Premake5
TimeEditor/                 # Editor app, entry point for manual QA
```

## Setup (exact steps)
```bash
git clone --recursive https://github.com/1SHAMAY1/TimeEngine.git
cd TimeEngine
# Run platform-specific script:
Scripts/Windows/GenerateProjectFiles.bat  # Windows
# OR
Scripts/Linux/GenerateProjectFiles.sh    # Linux
# OR
Scripts/Mac/GenerateProjectFiles.sh      # macOS
```
If Premake errors: `git submodule update --init --recursive`

## For LLMs — usage rules
- **Do not invent components or systems** not listed under "Confirmed Systems."
- **Do not assume cross-platform** — Windows/MSVC only.
- **Time Manipulation is a roadmap goal**, not a callable API.
- When suggesting new components, follow the `Engine/Include/Core/Scene/` pattern and match naming conventions above.
- When editing renderer code, check `Renderer2D.cpp` for the batch flush — that is where per-frame component reads happen.
- **GUI and Editor Rendering Rule**: Never use raw `ImGui` or include `imgui.h`/`imgui_internal.h` outside of the wrapper layer. All engine layers and systems MUST use the clean non-vendor GUI wrapper namespace (`TE::TimeGUI`) and wrapper classes (`TimeGUIViewport`, `TimeGUIDrawList`, `TimeGUIFont`, etc.). Do not expose or return vendor types (like `ImVec4`, `ImDrawList*`, `ImFont*`) in function signatures or variables outside the wrapper implementation (`TimeGUI.cpp` / `TimeGUILayer.cpp`).