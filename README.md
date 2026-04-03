# 🌀 Time Engine

**Time Engine** is a high-performance C++ game engine designed for sophisticated 2D and 3D application development. Built with a modular, data-driven architecture, it provides a professional-grade suite of tools for rendering, physics, and real-time editing.

<sc here>

---

## ✨ Features at a Glance

### 🖥️ Professional Editor Experience
*   **Refined Viewport Navigation**: Seamless WASD + Right-Click movement with real-time HUD feedback, camera speed scaling, and precise zoom-to-grid mechanics.
*   **Horizontal Viewport Toolbar**: Quick-access interface for switching between selection, transform, and specialized editor modes.
*   **Glass-like UI**: Modern, translucent context menus and property panels for a clean, non-intrusive workspace.
*   **Overhauled Hierarchy**: Standardized 34px row scaling with hierarchical icon font sizing for better scene organization.
*   **TEPropertyDrawer**: A modular system for generating consistent and stable property inspectors automatically.

<sc here>

### ⚡ Sprite Mode IDE (Procedural Engine)
A core refactor has introduced a fully data-driven, modular scripting environment within the engine:
*   **Recursive Expression Evaluator**: Supports complex nested math (e.g., `sin(a + b) * cos(c)`) with full operator precedence.
*   **Unified Block Executor**: A robust pipeline handling draw calls, assignments, and control flow (if/else, for-loops) in a single unified script.
*   **Dynamic Keyword Registry**: Variables are automatically registered as keywords upon assignment, enabling live code interaction.
*   **ISpriteLibrary Interface**: Decoupled rendering logic allowing for custom procedural extensions and plugins.

<vid here>

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

<sc here>

---

## 🤖 Automation & CI/CD
*   **Clang-Format Integration**: Automated code styling via GitHub Actions ensuring a consistent codebase.
*   **Merge Safety**: Workflows to prevent broken builds and maintain repository integrity.
*   **Integrated Logger**: Separate Core and Client logging channels for targeted debugging.

---

## 🚀 Getting Started

1.  **Clone**: `git clone https://github.com/1SHAMAY1/TimeEngine.git`
2.  **Generate**: Run `Scripts/GenerateProjectFiles.bat` to create the Visual Studio solution.
3.  **Build**: Open the solution and build the **Sandbox** project in `Debug` or `Release`.
4.  **Launch**: Run `Sandbox.exe` to access the Project Hub and start creating.

<vid here>

---

> [!NOTE]
> This engine is under active development. If you notice any area marked with `<sc here>` or `<vid here>` and want to contribute media, feel free to reach out!
