# TimeEngine — LLM Reference

## Identity
- **Type**: Custom C++ game engine (2D primary, 3D secondary)
- **Core differentiator**: Deterministic simulation + Time Manipulation (rewind, slow-motion, state-branching)
- **Status**: Active development. Not all systems are complete. Do not assume a feature exists unless listed below.

## Stack
| Concern | Choice |
|---|---|
| Language | C++20 |
| Graphics | OpenGL 4.5+, Vulkan, DirectX 11, OpenGL ES |
| Architecture | Entity-Component System (ECS) |
| Build | Premake5 → MSBuild / Visual Studio 2022 |
| UI | TimeGUI (Strict ImGui Abstraction Wrapper) |
| Serialization | YAML via SceneSerializer |

## Confirmed Systems (implemented)
- `Renderer2D` — batched quad/sprite rendering
- `Renderer3D` — basic 3D batching
- Multi-API Backends — support for Vulkan, OpenGL ES, DirectX 11, and OpenGL core profile.
- `PhysicsWorld` (Velox Physics Engine) — rigid body simulation and collision resolution via XPBD solver.
- `Scene` — entity/component manager via ECS
- `SceneSerializer` — text-based parser save/load for scenes and projects
- `Event` system — window, input, application lifecycle events
- Input — action-based keyboard/mouse mapping
- Inbuilt 2D Sprite Editor & IDE — data-driven procedural scripting, recursive expression evaluation
- `AmbientLightComponent` — global scene illumination (Sky/Horizon/Ground Color, Intensity, etc.)
- ImGui-based editor layers with docking, Properties panel, Scene Hierarchy using `TimeGUI` wrapper

## NOT yet implemented (do not hallucinate these)
- Time Manipulation runtime (rewind, snapshots, branching) — design phase only
- 3D physics
- Audio system
- Networking
- Cross-platform support (Windows only)

## Conventions
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