# TimeEngine `Engine/src` Subsystem Architecture Directory

This document serves as the master navigation index for all subsystem architectures located inside the [`Engine/src/`](file:///e:/TimeEngine/Engine/src) directory.

---

## Subsystem Architecture Index

Each link below redirects to the detailed, dedicated architecture document for that specific engine subsystem:

* ⚙️ **[Core Subsystem Architecture](file:///e:/TimeEngine/Engine/src/Core/ARCHITECTURE.md)** (`Engine/src/Core/`) — Central engine loop (`Application`), multi-threading pools, ECS (`Scene`, `EntityManager`), 2D physics, 2D collisions, asset management (`AssetManager`), game framework (`TObject`, `TComponent`), events, layer stack, and dynamic plugin loading.
* 🎨 **[Renderer Subsystem Architecture](file:///e:/TimeEngine/Engine/src/Renderer/ARCHITECTURE.md)** (`Engine/src/Renderer/`) — Multi-backend graphics abstraction (OpenGL, DX11, Vulkan, OpenGLES), 2D batching (`Renderer2D`, `RenderBatcher`), 2D lights, shadow volumes, materials, shaders, and framebuffers.
* 🛠️ **[Editor Subsystem Architecture](file:///e:/TimeEngine/Engine/src/Editor/ARCHITECTURE.md)** (`Engine/src/Editor/`) — Visual development workspace, editor modes (`EditorModeRegistry`), specialized asset editing tabs (`AssetEditorRegistry`), and toolbar controls.
* 🎯 **[Input Subsystem Architecture](file:///e:/TimeEngine/Engine/src/Input/ARCHITECTURE.md)** (`Engine/src/Input/`) — High-level action mapping context system (`InputSystem`, `InputAction`, `InputMappingContext`), priority contexts, and entity binding callbacks (`InputComponent`).
* 📷 **[Camera Subsystem Architecture](file:///e:/TimeEngine/Engine/src/Camera/ARCHITECTURE.md)** (`Engine/src/Camera/`) — 2D/3D camera matrix calculation (`Camera`), 2D orthographic projection (`OrthographicCamera`), 3D perspective projection (`PerspectiveCamera`), controllers, and player tracking (`PlayerCameraComponent`).
* 🖥️ **[Window Subsystem Architecture](file:///e:/TimeEngine/Engine/src/Window/ARCHITECTURE.md)** (`Engine/src/Window/`) — OS native window abstraction (`IWindow`), window creation (`WindowsWindow`), GLFW event hooks, VSync toggles, and swap chain buffer swapping.
* 🧰 **[Utils Subsystem Architecture](file:///e:/TimeEngine/Engine/src/Utils/ARCHITECTURE.md)** (`Engine/src/Utils/`) — Core engine toolbox, 2D engine math primitives (`MathUtils`), vendor-agnostic GUI wrappers (`TimeGUI`), and OS-specific platform file/registry utilities.


---

## Related Root Architecture References

- [Plugins Architecture](file:///e:/TimeEngine/Engine/Plugins/ARCHITECTURE.md) — Architecture for engine plugins and dynamic runtime extensions.
- [MCP Plugin Architecture](file:///e:/TimeEngine/Engine/Plugins/MCPPlugin/ARCHITECTURE.md) — Remote-control Model Context Protocol HTTP/SSE AI receiver plugin.
- [Windows Platform Utilities Architecture](file:///e:/TimeEngine/Engine/src/Utils/Platform/Windows/ARCHITECTURE.md) — Win32 file dialogs and registry helpers.
- [Unix Platform Utilities Architecture](file:///e:/TimeEngine/Engine/src/Utils/Platform/Unix/ARCHITECTURE.md) — POSIX executable resolution and Unix dialogs.
