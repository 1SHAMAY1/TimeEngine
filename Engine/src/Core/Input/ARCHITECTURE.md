# Core Low-Level Input Subsystem Architecture

The Core Input subsystem in TimeEngine provides low-level hardware device polling ([`Input`](../../../../Include/Input/Input.hpp)), key duration tracking (`InputState`), scroll delta accumulation, and key-code translation (`ToImGuiKey`).

> [!NOTE]
> In short, think of the **Core Input Subsystem** as the engine's hardware sensor reader: `Input` queries GLFW directly to see if keys or mouse buttons are physically pressed down, tracks how long keys are held, and converts keycodes for UI system compatibility.

---

## High-Level Action Input Reference Link

- [High-Level Action Mapping & InputSystem Architecture](../../Input/ARCHITECTURE.md) — Documentation for high-level `InputSystem`, `InputMappingContext`, `InputRemapper`, and `InputComponent`.

---

## Core Functions & Usage Rules

### 1. `Input::Init(nativeWindow)`
- **When Used**: Called internally during window creation (`WindowsWindow::Init`) to bind the OS window handle (`GLFWwindow*`).

---

## 2. Immediate Hardware Polling

#### `Input::IsKeyPressed(key)` & `Input::IsMouseButtonPressed(button)`
- **When Used**: Call inside low-level tools, camera controllers, or editor layers for immediate frame-by-frame state checks.

```cpp
if (TE::Input::IsKeyPressed(TE::KeyCode::W)) {
    // Move viewport camera forward
}
```

#### `Input::GetMousePosition()` / `GetMouseX()` / `GetMouseY()`
- **When Used**: Fetch current OS cursor position in window coordinates for picking or UI dragging.

#### `Input::GetMouseScrollDelta()`
- **When Used**: Query mouse wheel scroll delta ($X, Y$). Automatically resets delta values after query.

---

## 3. Key Duration & Event Tracking

#### `Input::OnKeyPressed(...)` & `Input::OnKeyReleased(...)`
- **When Used**: Invoked by event handlers to record `PressedTime` and compute `DurationHeld` in seconds inside `InputState`.

#### `Input::Update(deltaTime)`
- **When Used**: Called every frame inside `Application::Run()` update loop to accumulate `DurationHeld` on active held keys.

---

## 4. Keycode Translation (`Input::ToImGuiKey(key)`)
- **When Used**: Converts TimeEngine `TE::KeyCode` enum values to third-party `ImGuiKey` values for `TimeGUI` text input and shortcut navigation.

---

## Related Architectural Documentation

- [Input Subsystem Architecture](../../Input/ARCHITECTURE.md) — High-level `InputSystem` and `InputComponent` action mapping.
- [Window Subsystem Architecture](../../Window/ARCHITECTURE.md) — GLFW window creation and native input callback setup.
