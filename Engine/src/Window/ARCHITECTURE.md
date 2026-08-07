# Window Implementation Architecture

The Window subsystem in TimeEngine provides platform-agnostic OS window creation, event dispatching, graphics context binding, and input subsystem integration.

> [!NOTE]
> In short, think of the **Window Subsystem** as the physical glass frame for your game. When TimeEngine starts, it asks the Operating System (Windows/Mac/Linux) to open up a window box. It also acts as the engine's ears — whenever a player presses a key on their keyboard or moves their mouse, the Window system catches those physical gestures, wraps them into neat event envelopes, and passes them to the engine so the game can react instantly.


---

## Component Overview

- **Interface**: `IWindow` (`Engine/Include/Window/IWindow.hpp`) — Pure virtual abstract base class defining window operations.
- **Platform Implementation**: `WindowsWindow` (`Engine/Include/Window/WindowsWindow.hpp`) (`WindowsWindow.cpp`) — GLFW-backed desktop window implementation for Windows platforms.
- **Properties Struct**: `WindowProps` — Holds initial title, width, and height configuration.


## Architecture & Responsibilities

```
                      [ Application ]
                             │
                             ▼ (IWindow::Create)
                    ┌─────────────────┐
                    │     IWindow     │ (Interface)
                    └────────┬────────┘
                             │
                             ▼
                    ┌─────────────────┐
                    │  WindowsWindow  │ (GLFW Implementation)
                    └────────┬────────┘
                             │
       ┌─────────────────────┼─────────────────────┐
       ▼                     ▼                     ▼
[ Event Callbacks ]   [ Graphics API ]      [ Input System ]
(Window, Key, Mouse)  (OpenGL / DX11 /      (TE::Input::Init)
                      Vulkan context)
```

## Core Window Functions & Code Snippets

- **`IWindow::Create(props)`**: Factory function to instantiate the active platform window (`WindowsWindow`).
```cpp
IWindow* IWindow::Create(const WindowProps& props) {
    return new WindowsWindow(props);
}
```
- **`WindowsWindow::Init(props)`**: Initializes GLFW, configures graphic API hints (e.g. `GLFW_NO_API` for DX11/Vulkan), creates native GLFW window handle, and hooks event callbacks.
```cpp
void WindowsWindow::Init(const WindowProps &props) {
    glfwInit();
    m_Window = glfwCreateWindow(props.Width, props.Height, props.Title.c_str(), nullptr, nullptr);
    glfwSetWindowUserPointer(m_Window, &m_Data);
    TE::Input::Init(m_Window);
}
```
- **`WindowsWindow::OnUpdate()`**: Polls OS input/window events and presents/swaps render buffers (`glfwSwapBuffers` or DirectX 11 `SwapChain->Present`).
```cpp
void WindowsWindow::OnUpdate() {
    glfwPollEvents();
    if (TE::RendererContext::GetAPI() == TE::GraphicsAPI::DirectX11) {
        ctx.SwapChain->Present(m_Data.VSync ? 1 : 0, 0);
    } else {
        glfwSwapBuffers(m_Window);
    }
}
```
- **`WindowsWindow::SetVSync(enabled)`**: Controls vertical sync synchronization via `glfwSwapInterval(enabled ? 1 : 0)`.
- **GLFW Event Callbacks (`glfwSetKeyCallback`, `glfwSetCursorPosCallback`, etc.)**: Map native OS events to TimeEngine event objects and invoke `m_Data.EventCallback(event)`:
```cpp
## Window Subsystem Functions & Usage Rules

### 1. `IWindow::Create(props)`
- **When Used**: Called exactly once during application startup by `Application::Application()` to create the primary window.
- **When NOT to use**: Do NOT call `Create()` multiple times per application instance.

### 2. `WindowsWindow::Init(props)`
- **When Used**: Invoked internally by `WindowsWindow` constructor to setup GLFW hints, native window handle, and event callbacks.

### 3. `WindowsWindow::OnUpdate()`
- **When Used**: Called every single frame inside `Application::Run()` main loop.
- **What it does**: Processes pending OS window events (`glfwPollEvents`) and presents rendered frames to screen (`SwapChain->Present` or `glfwSwapBuffers`).

### 4. `WindowsWindow::SetVSync(enabled)`
- **When Used**: Preferred when toggling framerate synchronization in Engine Settings / Graphics settings UI.
- **Effect**: Locks render updates to display refresh rate (`1`), preventing screen tearing.



