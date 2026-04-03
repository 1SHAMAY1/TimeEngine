# 🌀 Time Engine

**Time Engine** is a high-performance C++ game engine designed for sophisticated 2D application development. Built with a modular, data-driven architecture, it provides a professional-grade suite of tools for rendering, physics, and real-time editing.

<img width="1264" height="701" alt="image" src="https://github.com/user-attachments/assets/d48599c2-300e-4cfb-9022-af682d60252f" />

---

## ✨ Core Engine Modules

### 🖥️ Professional Editor Experience
The Time Engine workspace is built for developer productivity, featuring:
*   **Viewport Navigation**: Seamless WASD + Right-Click movement with camera speed scaling and precise zoom-to-grid mechanics.
*   **Horizontal Viewport Toolbar**: Quick-access interface for switching between selection, transform, and specialized editor modes.
*   **TEPropertyDrawer**: A modular system for generating consistent and stable property inspectors automatically.
*   **Modern Glass UI**: Translucent context menus and property panels for a clean, non-intrusive workspace.

<img width="1266" height="698" alt="image" src="https://github.com/user-attachments/assets/8e2ae8d9-fd04-4680-a0cb-19b1c76fbfdd" />
<img width="1919" height="1089" alt="image" src="https://github.com/user-attachments/assets/e794acb5-7ada-4c5e-a673-c88d68d6be7c" />

### ⚡ Sprite Mode IDE (Procedural Engine)
A core refactor has introduced a fully data-driven, modular scripting environment within the engine:
*   **Recursive Expression Evaluator**: Supports complex nested math (e.g., `sin(a + b) * cos(c)`) with full operator precedence.
*   **Unified Block Executor**: A robust pipeline handling draw calls, assignments, and control flow (if/else, for-loops) in a single unified script.
*   **Live Code Interaction**: Automated variable registration for real-time scripting and visualization.

<img width="1919" height="1042" alt="image" src="https://github.com/user-attachments/assets/70832966-b5c1-4a41-aeb6-f8c45bc86754" />

https://github.com/user-attachments/assets/a91635ad-7f87-45c2-af39-fb66095ceb2b

---

## 🛠️ Component System (ECS)
The engine utilizes a modular ECS architecture. Below is the current development status and use case for each component:

### ✅ Tested & Stable
| Component | Use Case |
| :--- | :--- |
| **Box/Circle/Triangle/Polygon** | Procedural geometric shapes with integrated collision detection. Perfect for prototyping and geometric art. |
| **LightComponent** | Point and directional lights that interact with materials to create depth and atmosphere. |
| **TransformComponent** | Essential spatial data (X, Y, Rotation, Scale) for every world entity. |
| **TagComponent** | Unique identification for entity organization and retrieval. |

### 🚧 In Development (Not Fully Tested)
| Component | Use Case |
| :--- | :--- |
| **Sprite / Animated Sprite** | Texture rendering and flipbook-style animations for characters and environments. |
| **ProceduralSpriteComponent** | The base class for custom, code-driven visual elements via scripting. |
| **Input System** | Action-based input mapping for rebindable controls (Mouse/Keyboard). |
| **AmbientLightComponent** | Global illumination settings for the entire scene. |
| **ParallaxComponent** | Layered background scrolling to create a 2D sense of depth. |

### 🧪 Experimental / Early Modules
*   **Physics Engine (`PhysicsWorld`)**: Initial support for rigid body physics and simulation (not yet production-ready).
*   **Particle System**: Components for building emitters and managing particle pools for visual effects.

---

### 🎨 Rendering & Lighting
*   **OpenGL 4.5 Backend**: Utilizes modern rendering techniques for maximum efficiency.
*   **Batch Rendering**: Optimized draw calls for quads, sprites, and procedural primitives to handle millions of objects.
*   **Material System**: Flexible shader and texture management via a centralized library.

<img width="1919" height="1093" alt="image" src="https://github.com/user-attachments/assets/73eb1567-28cc-407b-bec5-3bb0841fad17" />

---

## 🚀 Future Roadmap
Our goal is to evolve Time Engine into a complete solution for 2D development:
1.  **Stability Pass**: Bringing all "In Development" modules to a "Tested & Stable" state.
2.  **Advanced Physics**: Enhancing the physics solver with constraints, ray-casting, and more complex body types.
3.  **Visual Effects**: Maturing the Particle System for fire, smoke, and environmental effects.
4.  **Asset Pipeline**: A more robust workflow for importing and managing external assets (Textures, Shaders, Audio).

---

## 🤝 Contributing
**Time Engine** is an open-source project, and we welcome contributions:
*   **💻 Code**: Optimizing the renderer, adding new components, or fixing bug reports.
*   **🎨 Media**: Providing high-quality screenshots and demonstration videos.
*   **📖 Documentation**: Improving the wiki and code comments.

---

## 🚀 Getting Started
1.  **Clone**: `git clone https://github.com/1SHAMAY1/TimeEngine.git`
2.  **Generate**: Run `Scripts/GenerateProjectFiles.bat` to create the Visual Studio solution.
3.  **Build**: Open the solution and build the **Sandbox** project in `Debug` or `Release`.
4.  **Launch**: Run `Sandbox.exe` to access the Project Hub and start creating.

---

> [!IMPORTANT]
> This engine is under **Active Development**. Modules marked as "In Development" or "Experimental" may undergo breaking changes.
