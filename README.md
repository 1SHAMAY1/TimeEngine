# Time Engine

**Time Engine** is a custom C++ game engine designed for 2D and 3D game development. It features a robust Editor, efficient rendering, and a modular architecture.

## 🌟 Currently Implemented Features

### �️ Project Hub
*   **Project Management**: Create new 2D or 3D projects.
*   **Recent Projects**: Quick access to previously opened projects.
*   **Project Generation**: Automates folder structure (`Assets`, `Scripts`) and pre-configurations.

### 🖥️ Editor Interface
*   **Docking System**: Fully customizable layout with dockable panels.
*   **Viewport**: Real-time rendering with grid overlay and camera navigation (WASD + Right Click).
*   **Scene Hierarchy**: Tree view of all entities in the active scene.
*   **Properties Panel**: Inspector for viewing and editing Entity components (Transform, Name).
*   **Content Browser**:
    *   Tabbed view for **Assets**, **Scripts**, and **Engine** resources.
    *   File type filtering (hides internal `.teproj` files).
    *   Thumbnail support for files and folders.
*   **Console**: Logging output for debugging (Core and Client loggers).

### ⚙️ Settings & Customization
*   **Project Settings**:
    *   Switch between **2D** and **3D** configurations.
    *   **2D Modes**: Supports **Top Down** (X/Y Axis) and **Side Scroller** (X/Z Axis) layouts using specific visual guides.
*   **Editor Settings**:
    *   **Theme Customization**: Real-time color editing for Window Bg, Headers, Buttons, and Tabs.
    *   **Camera Controls**: Adjustable Movement and Zoom speeds.
    *   **Navigation**: Toggle for physics collider visualization (placeholder) and navigation inputs.

### 🎨 Rendering (OpenGL)
*   **Core Renderer**: Setup for Shader management, Vertex Arrays, and Buffers.
*   **2D Rendering**: Efficient Quad and Sprite rendering.
*   **Framebuffer**: Viewport rendering to ImGui image.
*   **Texture Support**: Loading and binding of textures (PNG/JPG).

### 🎬 Scene System
*   **Entity-Component System (ECS)**:
    *   **Entity**: Base game object with a unique ID.
    *   **Components**: Transform (Position, Rotation, Scale), Tag.
*   **Serialization**: YAML-based project and scene saving/loading.

### 🎮 Input & Events
*   **Event System**: Robust event dispatching for Window, Key, and Mouse events.
*   **Input Polling**: Static access to keyboard and mouse state.

---

## 🚀 Getting Started
1.  Clone the repository.
2.  Run `Scripts/GenerateProjectFiles.bat` to generate the Visual Studio solution.
3.  Build the **Sandbox** project in `Debug` or `Release` mode.
4.  Launch `Sandbox.exe` to open the Project Hub.
