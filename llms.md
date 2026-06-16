# TimeEngine — LLM Reference

## Identity
- **Type**: Custom C++ game engine (2D primary, 3D secondary)
- **Core differentiator**: Deterministic simulation + Time Manipulation (rewind, slow-motion, state-branching)
- **Status**: Active development. Not all systems are complete. Do not assume a feature exists unless listed below.

## Stack
| Concern | Choice |
|---|---|
| Language | C++20 |
| Graphics | OpenGL 4.5+ core profile |
| Architecture | Entity-Component System (ECS) |
| Build | Premake5 → MSBuild / Visual Studio 2022 |
| UI | ImGui (custom docking + editor layers) |
| Serialization | YAML via SceneSerializer |

## Confirmed Systems (implemented)
- `Renderer2D` — batched quad/sprite rendering
- `Renderer3D` — basic 3D batching
- `Scene` — entity/component manager via ECS
- `SceneSerializer` — text-based parser save/load for scenes and projects
- `Event` system — window, input, application lifecycle events
- Input — action-based keyboard/mouse mapping
- Inbuilt 2D Sprite Editor & IDE — data-driven procedural scripting, recursive expression evaluation
- `AmbientLightComponent` — global scene illumination (Sky/Horizon/Ground Color, Intensity, etc.)
- ImGui editor with docking, Properties panel, Scene Hierarchy

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

## File Layout (partial — extend as codebase grows)
```
Engine/
  Include/
    Core/Scene/           # ECS components live here (e.g. AmbientLightComponent.hpp)
    Renderer/             # Renderer headers
    Layers/               # EditorLayer.hpp, etc.
  src/
    Core/Scene/           # Scene.cpp, SceneSerializer.cpp, EntityManager.cpp
    Renderer/             # Renderer2D.cpp, etc.
    Core/Layers/          # EditorLayer.cpp, etc.
Scripts/
  GenerateProjectFiles.bat  # Run first — generates .sln via Premake5
Sandbox/                  # Test app, entry point for manual QA
```

## Setup (exact steps)
```bash
git clone --recursive https://github.com/1SHAMAY1/TimeEngine.git
cd TimeEngine
Scripts/GenerateProjectFiles.bat   # generates TimeEngine.sln
# Open TimeEngine.sln in VS2022, build Sandbox, press F5
```
If Premake errors: `git submodule update --init --recursive`

## For LLMs — usage rules
- **Do not invent components or systems** not listed under "Confirmed Systems."
- **Do not assume cross-platform** — Windows/MSVC only.
- **Time Manipulation is a roadmap goal**, not a callable API.
- When suggesting new components, follow the `Engine/Include/Core/Scene/` pattern and match naming conventions above.
- When editing renderer code, check `Renderer2D.cpp` for the batch flush — that is where per-frame component reads happen.