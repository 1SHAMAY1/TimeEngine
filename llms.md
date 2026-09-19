# TimeEngine — LLM Reference

## Identity
- **Type**: Custom C++ game engine and Integrated Game Development Environment (2D primary, 3D secondary)
- **Core differentiator**: Deterministic simulation + Time Manipulation (rewind, slow-motion, state-branching)
- **Status**: Active development. Not all systems are complete. Do not assume a feature exists unless listed below.

## Core Architecture
- **Language**: Modern C++ (C++Latest / C++20)
- **Graphics API**: OpenGL 4.5+ DSA, Vulkan 1.3, DirectX 11, OpenGL ES 3.0, Metal
- **Architecture**: Entity-Component System (ECS)
- **Build System**: Premake5 / MSBuild / GNU Make (Tiered Parallel Pipeline)
- **UI**: TimeGUI (Strict vendor-insulated abstraction wrapper)
- **Documentation**: Root portal (`index.html`) & interactive docs reader (`Website/docs.html`)

## Key APIs & Systems
- **Renderer**: `Renderer2D` and `Renderer3D` (optimized batching for quads/sprites). Supports Vulkan, OpenGL ES, DirectX 11, Metal, and OpenGL core profile.
- **Camera Subsystem**: `Camera`, `OrthographicCamera`, `PerspectiveCamera`, `CameraController`, and `PlayerCameraComponent`.
- **Physics**: `PhysicsWorld` (Velox Physics Engine) — rigid body simulation and collision resolution via XPBD solver.
- **Inbuilt 2D Sprite Editor & IDE**: Data-driven procedural scripting with recursive expression evaluation.
- **Scene System**: `Scene` class manages entities and components via ECS.
- **Serialization**: Scene (`.tescene`), Project (`.teproj`), Material (`.tematerial`), Texture (`.tetexture`), and Plugin (`.teplugin`) serializations.
- **Events**: Event systems for windowing, user input (`ApplicationEvent`, `KeyEvent`, `MouseEvent`), and type-safe `EventDispatcher`.
- **Input**: High-level action-based mapping contexts (`InputSystem`, `InputAction`, `InputMappingContext`).
- **Plugins Settings Panel**: Built-in GUI panel (toggled via `Edit -> Plugins`) showing discovered plugins and enabling runtime load/unload DLL manipulation.
- **MCP Automation & SSE Server**: Built-in server providing remote programmatic automation:
  - Port: `3000` (HTTP and SSE streams `/message`).
  - Active Tools: `get_engine_info`, `get_scene_hierarchy`, `create_entity`, `destroy_entity`, `add_component`, `set_entity_properties`, `get_viewport_screenshot`, `delete_screenshot`, `send_editor_input`, `create_directory`, `delete_file_or_directory`.
- **Automated Testing Suite**: Headless test runner (`HeadlessTestRunner`), unit, integration, and stress tests executed via `Scripts/Windows/RunTests.bat` and `Scripts/Windows/RunStressTests.bat`.

## Development Patterns & Engine Types
- Use `TE_CORE_LOG` (`TE_CORE_INFO`, `TE_CORE_WARN`, `TE_CORE_ERROR`) for engine-side logging.
- Use `TE_CLIENT_LOG` (`TE_CLIENT_INFO`, `TE_CLIENT_WARN`, `TE_CLIENT_ERROR`) for sandbox/game logging.
- **Memory Management (No Raw Owning Pointers)**:
  - Never use raw owning pointers (`new`/`delete` or unmanaged pointers).
  - Use `TE::Scope<T>` / `CreateScope<T>(...)` for unique/exclusive ownership (`std::unique_ptr`).
  - Use `TE::Ref<T>` / `CreateRef<T>(...)` for shared ownership (`std::shared_ptr`).
  - Use `TE::WeakRef<T>` for non-owning observers (`std::weak_ptr`).
- **Engine Strings**:
  - Always use `TEString` (from `Core/TEString.hpp` via `Core/PreRequisites.h`) and `TEStringView` instead of `std::string` or `const char*` across engine APIs, ECS components, and properties.
- **Gameplay Containers & Utilities**:
  - Use `TEArray<T>`, `TEOption<T>`, `TESpan<T>`, and `TEResult<T, E>` (from `Framework/Gameplay/Include/GameplayUtils.hpp`).
- **Math & Vectors**:
  - Always use `TEVector2`, `TEVector`, `TEVector4`, `TEMatrix4`, `TEQuat` (from `Runtime/Math/Include/MathUtils.hpp`) instead of raw GLM or vendor math structs.
- All headers MUST include `#pragma once`.
- Classes: `PascalCase`, variables: `camelCase` (`m_` prefix for private members), macros: `SCREAMING_SNAKE_CASE` (`TE_`).
- Components live under `Engine/Source/Runtime/Scene/Include/` and register via macros (e.g. `T_REGISTER_COMPONENT`).

## Setup
1. Run the workspace generation script in `Scripts/` (e.g. `Scripts/Windows/GenerateProjectFiles.bat` for Windows).
2. Build with platform build scripts (e.g. `Scripts/Windows/MSVC/BuildDebug.bat`).
3. Output binaries are generated cleanly in `Artifacts/Bin/` and `Artifacts/Bin-Intermediate/`.
4. Launch TimeEditor to access the Project Hub.

| Pattern | Rule |
|---|---|
| Logging (engine) | `TE_CORE_LOG` (`TE_CORE_INFO`, `TE_CORE_WARN`, `TE_CORE_ERROR`) |
| Logging (game/sandbox) | `TE_CLIENT_LOG` (`TE_CLIENT_INFO`, `TE_CLIENT_WARN`, `TE_CLIENT_ERROR`) |
| Memory Management | `Scope<T>` / `CreateScope<T>`, `Ref<T>` / `CreateRef<T>`, `WeakRef<T>` (NO raw owning pointers) |
| Strings | `TEString`, `TEStringView` (NO raw `std::string` in engine interfaces) |
| Containers & Views | `TEArray<T>`, `TEOption<T>`, `TESpan<T>`, `TEResult<T, E>` |
| Math & Vectors | `TEVector2`, `TEVector`, `TEVector4`, `TEMatrix4` (from `MathUtils.hpp`) |
| Headers | `#pragma once` always |
| Naming | PascalCase classes, camelCase variables (`m_` for privates), SCREAMING_SNAKE_CASE macros |
| Formatting | `.clang-format` enforced, 4-space indent |

## ThirdParty Wrapper Mapping
All third-party libraries have strict wrappers that isolate raw vendor classes, headers, and APIs. Direct calls to vendor headers in engine code are prohibited.

- **ImGui / ForgeUI (UI/Editor Layouts)**:
  - Wrapper Headers: [TimeGUI.hpp](file:///E:/TimeEngine/Engine/Source/Runtime/UI/Include/TimeGUI.hpp)
  - Wrapper Sources: [TimeGUI.cpp](file:///E:/TimeEngine/Engine/Source/Runtime/UI/Source/TimeGUI.cpp)
- **OpenGL/Glad/OpenGLES/Vulkan/DirectX11/Metal (Graphics/Renderer)**:
  - Batched renderer: [Renderer2D.hpp](file:///E:/TimeEngine/Engine/Source/Runtime/Renderer2D/Include/Renderer2D.hpp)
  - Generic Renderer API: [RendererAPI.hpp](file:///E:/TimeEngine/Engine/Source/Runtime/RHI/Include/RendererAPI.hpp)
  - Shaders: [Shader.hpp](file:///E:/TimeEngine/Engine/Source/Runtime/RHI/Include/Shader.hpp)
  - Textures: [Texture.hpp](file:///E:/TimeEngine/Engine/Source/Runtime/RHI/Include/Texture.hpp)
  - Colors: [TEColor.hpp](file:///E:/TimeEngine/Engine/Source/Runtime/Math/Include/TEColor.hpp)
- **Velox (Physics Engine)**:
  - Abstraction: [PhysicsWorld.hpp](file:///E:/TimeEngine/Engine/Source/Runtime/Physics/Include/PhysicsWorld.hpp)
- **GLFW (Windowing & OS Input Integration)**:
  - OS abstraction interface: [IWindow.hpp](file:///E:/TimeEngine/Engine/Source/Runtime/Window/Include/IWindow.hpp)
  - Windows implementation: [WindowsWindow.cpp](file:///E:/TimeEngine/Engine/Source/Runtime/Window/Source/WindowsWindow.cpp)
- **Customizable Logger (Logging Backend)**:
  - System logging interface: [Log.h](file:///E:/TimeEngine/Engine/Source/Runtime/Core/Include/Log.h)
- **GLM (Mathematics / Vectors / Matrices)**:
  - Math & Vectors wrapper: [MathUtils.hpp](file:///E:/TimeEngine/Engine/Source/Runtime/Math/Include/MathUtils.hpp)
- **stb_image / Asset Importers (Image & Model Loaders)**:
  - Asset Manager: [AssetManager.hpp](file:///E:/TimeEngine/Engine/Source/Framework/Asset/Include/AssetManager.hpp)

## Modular File Layout
```
TimeEngine/
├── Artifacts/              # Centralized build output (Bin/, Bin-Intermediate/)
├── Docs/                   # Documentation & in-IDE Solution Project
├── Engine/
│   ├── Source/
│   │   ├── Runtime/        # Core runtime modules (Core, Math, RHI, Renderer2D, Physics, Scripting, Scene, Testing, UI, Window)
│   │   └── Framework/      # High-level engine systems (Asset, Input, Gameplay, Application)
│   └── premake5.lua
├── Plugins/                # Decoupled marketplace plugins (Shipping/ and Experimental/)
├── Resources/              # Shared engine resources (Branding, Editor, Templates)
├── Scripts/                # Cross-platform build & test scripts
├── ThirdParty/             # Insulated vendor submodules (Velox, GLM, GLFW, ImGui, ForgeUI, Logger)
└── TimeEditor/             # IGDE Application source & resources
```
