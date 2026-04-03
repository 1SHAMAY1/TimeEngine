# 🌀 Time Engine

**Time Engine** is a high-performance C++ game engine designed for sophisticated 2D application development. Built with a modular, data-driven architecture, it provides a professional-grade suite of tools for rendering, physics, and real-time editing.

<img width="1264" height="701" alt="image" src="https://github.com/user-attachments/assets/d48599c2-300e-4cfb-9022-af682d60252f" />

---

## ✨ Features at a Glance

### 🖥️ Professional Editor Experience
*   **Refined Viewport Navigation**: Seamless WASD + Right-Click movement with real-time HUD feedback, camera speed scaling, and precise zoom-to-grid mechanics.
*   **Horizontal Viewport Toolbar**: Quick-access interface for switching between selection, transform, and specialized editor modes.
*   **Glass-like UI**: Modern, translucent context menus and property panels for a clean, non-intrusive workspace.
*   **Overhauled Hierarchy**: Standardized 34px row scaling with hierarchical icon font sizing for better scene organization.
*   **TEPropertyDrawer**: A modular system for generating consistent and stable property inspectors automatically.

<img width="1266" height="698" alt="image" src="https://github.com/user-attachments/assets/8e2ae8d9-fd04-4680-a0cb-19b1c76fbfdd" />
<img width="1919" height="1089" alt="image" src="https://github.com/user-attachments/assets/e794acb5-7ada-4c5e-a673-c88d68d6be7c" />

### ⚡ Sprite Mode IDE (Procedural Engine)
A core refactor has introduced a fully data-driven, modular scripting environment within the engine:
*   **Recursive Expression Evaluator**: Supports complex nested math (e.g., `sin(a + b) * cos(c)`) with full operator precedence.
*   **Unified Block Executor**: A robust pipeline handling draw calls, assignments, and control flow (if/else, for-loops) in a single unified script.
*   **Dynamic Keyword Registry**: Variables are automatically registered as keywords upon assignment, enabling live code interaction.
*   **ISpriteLibrary Interface**: Decoupled rendering logic allowing for custom procedural extensions and plugins.

<img width="1919" height="1042" alt="image" src="https://github.com/user-attachments/assets/70832966-b5c1-4a41-aeb6-f8c45bc86754" />

https://github.com/user-attachments/assets/a91635ad-7f87-45c2-af39-fb66095ceb2b



### 🛠️ Advanced ECS & Physics
*   **Component Suite**: Beyond basic transforms, the engine now supports:
    *   **Colliders**: Box, Circle, Triangle, and complex **Polygon Colliders**.
    *   **Rendering**: Animated Sprites, Procedural Sprites, and Parallax layers.
    *   **Lighting**: Ambient and Point/Directional light components.
*   **Broad-Phase Collision**: Optimized intersection testing for high-entity counts.
*   **Serialization**: Robust YAML-based scene and project serialization for easy save/load workflows.

<vid here>

### 🎨 Rendering & Lighting
*   **OpenGL 4.5 Backend**: Utilizes modern rendering techniques for maximum efficiency.
*   **2D Lighting System**: Refined lighting refinements including ambient light stabilization and per-object material properties.
*   **Batch Rendering**: Optimized draw calls for quads, sprites, and procedural primitives.
*   **Material System**: Flexible shader and texture management via a centralized library.

<img width="1919" height="1093" alt="image" src="https://github.com/user-attachments/assets/73eb1567-28cc-407b-bec5-3bb0841fad17" />

---

## 🤝 Contributing

**Time Engine** is an open-source project, and we welcome contributions from the community! Whether you are a developer, artist, or designer, your help is appreciated:

*   **💻 Code & Features**: Optimizing the renderer, adding new ECS components, or porting to other platforms.
*   **🎨 Media & Documentation**: Providing high-quality screenshots (`<sc here>`), demonstration videos (`<vid here>`), or improving the wiki.
*   **🐛 Bug Reports**: Reporting issues or suggesting new ideas through the GitHub Issues tab.

---

## 🚀 Getting Started

1.  **Clone**: `git clone https://github.com/1SHAMAY1/TimeEngine.git`
2.  **Generate**: Run `Scripts/GenerateProjectFiles.bat` to create the Visual Studio solution.
3.  **Build**: Open the solution and build the **Sandbox** project in `Debug` or `Release`.
4.  **Launch**: Run `Sandbox.exe` to access the Project Hub and start creating.

<vid here>

---

> [!IMPORTANT]
> This engine is under **Active Development**. We are constantly adding new modules and refining the core. If you'd like to contribute code or media, feel free to open a Pull Request or reach out!
