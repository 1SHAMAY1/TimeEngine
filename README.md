<p align="center">
<img width="130" height="130" alt="TimeEngineIcon" src="https://github.com/user-attachments/assets/97592994-22ae-4371-8024-3b3c35344666" />
</p>

<h1 align="center">Time Engine</h1>

<p align="center">
  <strong>A high-performance C++ game engine designed for sophisticated 2D application development and deterministic time manipulation.</strong>
</p>

<p align="center">
  <img src="https://img.shields.io/badge/Language-C%2B%2B17-blue.svg" alt="C++17">
  <img src="https://img.shields.io/badge/Platform-Windows%20%7C%20Linux%20%7C%20macOS-lightgrey.svg" alt="Windows | Linux | macOS">
  <img src="https://img.shields.io/badge/Graphics-OpenGL%20%7C%20Vulkan%20%7C%20DirectX%2011%20%7C%20OpenGL%20ES-orange.svg" alt="Graphics">
  <img src="https://img.shields.io/badge/License-MIT-green.svg" alt="License MIT">
</p>

<p align="center">
  <img width="1264" height="701" alt="Time Engine Editor Workspace" src="https://github.com/user-attachments/assets/d48599c2-300e-4cfb-9022-af682d60252f" />
</p>

---

## ⏳ The Core Vision: Time Manipulation

TimeEngine is built with a unique architectural goal: **deterministic simulation**.
While it provides a professional-grade suite for 2D development, it is being evolved to support:
- **Frame-Perfect Rewind**: Snapshot-based state management for instant time reversal.
- **Deterministic Logic**: Ensuring simulation consistency across environments.
- **Time Dilation**: Granular control over simulation speed and flow.

*Note: Time manipulation features are currently in active development. See our [Roadmap](ROADMAP.md).*

---

## ✨ Core Engine Modules

### 🖥️ Professional Editor Experience
The Time Engine workspace is built for developer productivity, featuring:
*   **Viewport Navigation**: Seamless WASD + Right-Click movement with camera speed scaling and precise zoom-to-grid mechanics.
*   **Horizontal Viewport Toolbar**: Quick-access interface for switching between selection, transform, and specialized editor modes.
*   **TEPropertyDrawer**: A modular system for generating consistent and stable property inspectors automatically.
*   **Modern Glass UI**: Translucent context menus and property panels for a clean, non-intrusive workspace.
*   **Plugins Settings Panel**: A built-in GUI panel (via `Edit -> Plugins`) to dynamically view discovered plugins, load/unload DLLs, toggle their active state, and write persistent enabled properties to disk.
*   **Advanced Performance Profiler**: High-fidelity profiling dashboard showing frame timing breakdowns (Game update, Render flush, Physics simulation, UI render) visualized through stacked horizontal bars, live scrolling plots, and dynamic Heap/Stack memory allocation tables (tracking object instances and active call stack frames).
*   **MCP Automation Interface**: A built-in SSE + JSON-RPC network automation interface allowing full, programmatic remote control of editor tools, screenshot retrieval, input simulation, and component properties editing.

<p align="center">
  <img width="600" alt="Editor UI 1" src="https://github.com/user-attachments/assets/8e2ae8d9-fd04-4680-a0cb-19b1c76fbfdd" />
  <img width="600" alt="Editor UI 2" src="https://github.com/user-attachments/assets/e794acb5-7ada-4c5e-a673-c88d68d6be7c" />
</p>

### ⚡ Inbuilt 2D Sprite Editor & IDE (Final Stages)
A core refactor has introduced a fully data-driven, modular scripting environment within the engine:
*   **Recursive Expression Evaluator**: Supports complex nested math (e.g., `sin(a + b) * cos(c)`) with full operator precedence.
*   **Unified Block Executor**: A robust pipeline handling draw calls, assignments, and control flow (if/else, for-loops) in a single unified script.
*   **Live Code Interaction**: Automated variable registration for real-time scripting and visualization.

<p align="center">
<img width="1919" height="1131" alt="image" src="https://github.com/user-attachments/assets/ee8db0e9-4f17-4b33-b2a4-bfd60151338a" />
<img width="1919" height="1138" alt="image" src="https://github.com/user-attachments/assets/0bbb0594-019a-4173-8ade-cfdd333836f4" />
<img width="1919" height="1134" alt="image" src="https://github.com/user-attachments/assets/a6b2bbd7-bdfe-4817-ab3c-6d2ac21d020e" />
<img width="1919" height="1135" alt="image" src="https://github.com/user-attachments/assets/9d5f8098-c56d-4514-9f8f-f80dec346490" />

</p>


---

## 🛠️ Component System (ECS)
The engine utilizes a modular ECS architecture. Below is the current development status and use case for each component:

### ✅ Tested & Stable
| Component | Use Case |
| :--- | :--- |
| **Box/Circle/Triangle/Polygon** | Procedural geometric shapes with integrated collision detection. Perfect for prototyping and geometric art. |
| **LightComponent** | Point and directional lights that interact with materials to create depth and atmosphere. |
| **TransformComponent** | Essential spatial data (X, Y, Rotation, Scale) for every entity. |
| **TagComponent** | Unique identification for entity organization and retrieval. |

### 🛠️ Nearly Stable (Final Testing)
| Component | Use Case |
| :--- | :--- |
| **Sprite / Animated Sprite** | Texture rendering and flipbook-style animations for characters and environments. |
| **ProceduralSpriteComponent** | The base class for custom, code-driven visual elements via scripting. |

### 🚧 In Development
| Component | Use Case |
| :--- | :--- |
| **Input System** | Action-based input mapping for rebindable controls (Mouse/Keyboard). |
| **AmbientLightComponent** | Global illumination settings for the entire scene. |
| **ParallaxComponent** | Layered background scrolling to create a 2D sense of depth. |

### 🧪 Experimental / Early Modules
*   **Physics Engine (`PhysicsWorld`)**: Initial support for rigid body physics and simulation (not yet production-ready).
*   **Particle System**: Components for building emitters and managing particle pools for visual effects.

---

### 🎨 Rendering & Graphics Backends
*   **OpenGL 4.5**: Modern desktop rendering with DSA (Direct State Access) for maximum efficiency on Windows, Linux, and macOS.
*   **Vulkan**: Low-overhead, explicit GPU API for high-performance rendering with full control over the graphics pipeline.
*   **DirectX 11**: Native Windows rendering backend via D3D11 for broad hardware compatibility on the Windows platform.
*   **OpenGL ES**: Lightweight ES backend targeting embedded and mobile-class hardware profiles.
*   **Batch Rendering**: Optimized draw calls for quads, sprites, and procedural primitives across all backends.
*   **Material System**: Flexible shader and texture management via a centralized, backend-agnostic library.

<p align="center">
  <img width="1264" alt="Rendering Demo" src="https://github.com/user-attachments/assets/73eb1567-28cc-407b-bec5-3bb0841fad17" />
</p>

---

## 🚀 Getting Started

1.  **Clone**: `git clone --recursive https://github.com/1SHAMAY1/TimeEngine.git`
2.  **Generate**: Run the workspace generation script for your platform:
    *   **Windows**: `Scripts/Windows/GenerateProjectFiles.bat`
    *   **Linux**: `Scripts/Linux/GenerateProjectFiles.sh`
    *   **macOS**: `Scripts/Mac/GenerateProjectFiles.sh`
3.  **Build**: Open the generated IDE workspace/solution or run the build command for your platform:
4.  **Launch**: Run `TimeEditor.exe` to access the Project Hub.

---

## 🤖 LLM-Assisted Development

TimeEngine is optimized for development with AI coding assistants (e.g., Gemini, Claude, Cursor). We provide structured context files to help LLMs understand our architecture, folder structure, and vendor wrapper constraints immediately:

*   [llms.md](llms.md) / [llms.txt](llms.txt): Detailed mapping of project architectures, directory structures, vendor library wrapping rules (e.g., `TimeGUI` wrapper for ImGui, `MathUtils` for GLM), and coding conventions.
*   [.agents/rules/priority.md](.agents/rules/priority.md): System-level instructions loaded by agentic frameworks to enforce coding standards automatically.

---

## 🤝 Contributing
**Time Engine** is an open-source project, and we welcome contributions. Please see [CONTRIBUTING.md](CONTRIBUTING.md) and our [Roadmap](ROADMAP.md) for more details.

---

> [!IMPORTANT]
> This engine is under **Active Development**. Modules marked as "In Development" or "Experimental" may undergo breaking changes.

