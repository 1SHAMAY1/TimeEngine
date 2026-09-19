# Window Subsystem Architecture

The **Window Subsystem** in TimeEngine provides modular, platform-agnostic OS window creation, event dispatching, graphics context binding, and input subsystem integration.

> [!NOTE]
> In short, think of the **Window Subsystem** as the physical glass frame for your game. When TimeEngine starts, it asks the Operating System (Windows, macOS, or Linux) to open up a window box. It also acts as the engine's ears — whenever a player presses a key on their keyboard or moves their mouse, the Window system catches those physical gestures, wraps them into neat event envelopes, and passes them to the engine so the game can react instantly.

---

## 🏛️ Architecture & Platform Implementations

`mermaid
graph TD
    A[Application::Application()] -->|Calls IWindow::Create(props)| B[IWindow (Abstract Interface)]
    B -->|#ifdef TE_PLATFORM_WINDOWS| C[WindowsWindow (Win32 / GLFW)]
    B -->|#elif defined(TE_PLATFORM_LINUX)| D[LinuxWindow (X11 / Wayland / GLFW)]
    B -->|#elif defined(TE_PLATFORM_MACOS)| E[MacWindow (Cocoa / GLFW)]
    C --> F[Event Callbacks / Graphics API / Input System]
    D --> F
    E --> F
`

---

## 🔑 Core Components

### 1. IWindow Interface
Defined in [Engine/Include/Window/IWindow.hpp](file:///e:/TimeEngine/Engine/Include/Window/IWindow.hpp):
- Pure virtual abstract interface defining the contract for OS windows.
- Declares GetWidth(), GetHeight(), SetVSync(), IsVSync(), OnUpdate(), GetNativeWindow(), GetGLLoaderFunction().
- Static factory: static TEScope<IWindow> Create(const WindowProps &props) defined in [IWindow.cpp](file:///e:/TimeEngine/Engine/src/Window/IWindow.cpp).

### 2. Concrete Platform Window Classes
- **WindowsWindow** ([Include/Window/WindowsWindow.hpp](file:///e:/TimeEngine/Engine/Include/Window/WindowsWindow.hpp) / [src/Window/WindowsWindow.cpp](file:///e:/TimeEngine/Engine/src/Window/WindowsWindow.cpp)):
  - Dedicated Microsoft Windows implementation.
  - Handles Win32 native icons (LoadImageW, WM_SETICON), DirectX11RendererAPI::InitWithWindow interop via glfwGetWin32Window, and OpenGL context management.
- **LinuxWindow** ([Include/Window/LinuxWindow.hpp](file:///e:/TimeEngine/Engine/Include/Window/LinuxWindow.hpp) / [src/Window/LinuxWindow.cpp](file:///e:/TimeEngine/Engine/src/Window/LinuxWindow.cpp)):
  - Dedicated Linux implementation (X11/Wayland).
  - Handles OpenGL and Vulkan window initialization and Linux event dispatching.
- **MacWindow** ([Include/Window/MacWindow.hpp](file:///e:/TimeEngine/Engine/Include/Window/MacWindow.hpp) / [src/Window/MacWindow.cpp](file:///e:/TimeEngine/Engine/src/Window/MacWindow.cpp)):
  - Dedicated Apple macOS implementation (Cocoa).
  - Configures macOS-specific OpenGL Core Profile hints (GLFW_CONTEXT_VERSION_MAJOR 4, 1, GLFW_OPENGL_FORWARD_COMPAT) and Vulkan/Metal presentation.

---

## ⚡ Factory Creation Pattern

In [Engine/src/Window/IWindow.cpp](file:///e:/TimeEngine/Engine/src/Window/IWindow.cpp):
`cpp
TEScope<IWindow> IWindow::Create(const WindowProps &props)
{
#ifdef TE_PLATFORM_WINDOWS
    return CreateScope<WindowsWindow>(props);
#elif defined(TE_PLATFORM_LINUX)
    return CreateScope<LinuxWindow>(props);
#elif defined(TE_PLATFORM_MACOS)
    return CreateScope<MacWindow>(props);
#else
    TE_CORE_ASSERT(false, Unknown platform for IWindow::Create!);
    return nullptr;
#endif
}
`

---

## 🛠️ Usage Rules & Lifecycle

1. **IWindow::Create(props)**:
   - Called once during engine startup in Application::Application().
   - Returns a TEScope<IWindow> holding the active platform window.
2. **OnUpdate()**:
   - Called once per frame inside Application::Run() to poll OS events via glfwPollEvents().
3. **SetVSync(enabled)**:
   - Controls vertical sync refresh throttling via glfwSwapInterval(enabled ? 1 : 0).
