# Input Subsystem Architecture

The Input subsystem in TimeEngine handles raw device polling (keyboard, mouse), event state tracking, and high-level context-based action mapping (`InputSystem` & `InputComponent`).

> [!NOTE]
> In short, think of the **Input Subsystem** as the game's nervous system: `Input` static class handles low-level polling (asking "is Space pressed right now?"); while `InputSystem` and `InputComponent` convert raw key presses into high-level game actions (like "Jump" or "MoveRight") that game entities can respond to cleanly.

---

## Architecture & Data Flow

```
[ OS Hardware Input ]
         │
         ▼ (GLFW Callbacks in WindowsWindow)
[ Input Static Class ]  ◄── (Raw Polling: Input::IsKeyPressed)
         │
         ▼ (Evaluates Context Priorities)
[ InputSystem Manager ]
         │
         ▼ (Triggers Action Bindings)
[ InputComponent ] ─────► [ Game Entity Callbacks ]
```

---

## Subsystem Components

1. **Low-Level Device Polling (`Input` — `Engine/Include/Input/Input.hpp`)**:
   - Static class maintaining hardware states (`s_KeyStates`, `s_MouseStates`, scroll deltas).
   - Serves immediate polling queries from anywhere in the codebase.

2. **High-Level Action Mapping System**:
   - **`InputAction` (`Engine/Include/Input/InputAction.hpp`)**: Represents abstract actions (`Jump`, `Fire`, `Move2D`) with value types (`Digital` (bool), `Axis1D` (float), `Axis2D` (`TEVector2`)).
   - **`InputMappingContext` (`Engine/Include/Input/InputMappingContext.hpp`)**: Map of keys to `InputAction` objects for specific gameplay states (e.g. `InGameContext`, `VehicleContext`).
   - **`InputRemapper` (`Engine/Include/Input/InputRemapper.hpp`)**: Handles runtime key-rebinding overrides.
   - **`InputSystem` (`Engine/Include/Input/InputSystem.hpp`)**: Singleton manager processing priority-sorted contexts each frame and executing bindings.
   - **`InputComponent` (`Engine/Include/Input/InputComponent.hpp`)**: ECS component attached to game entities to register callback functions (`BindAction`).

---

## Function Roles & Usage Guidelines

### 1. Low-Level Polling (`Input`)

#### `Input::IsKeyPressed(key)` / `Input::IsMouseButtonPressed(button)`
- **When Used**: Call in editor tools, player movement scripts, or frame-by-frame updates when immediate key state polling is needed.
- **When NOT to use**: Avoid spreading hardcoded `KeyCode` checks across high-level gameplay scripts (use `InputComponent` action bindings instead).

```cpp
if (TE::Input::IsKeyPressed(TE::KeyCode::Space)) {
    // Jump immediate action
}
```

#### `Input::GetMousePosition()` / `Input::GetMouseX()` / `Input::GetMouseY()`
- **When Used**: Call when converting screen coordinates to world coordinates or checking cursor positions for UI picking.

#### `Input::GetMouseScrollDelta()`
- **When Used**: Call inside viewport camera controls to handle zoom-in / zoom-out behavior.

---

### 2. High-Level Action Mapping (`InputSystem` & `InputComponent`)

#### `InputSystem::Get().AddMappingContext(context, priority)`
- **When Used**: Call when enabling a set of controls (e.g., entering a vehicle or opening a menu).
- **Priority**: Higher priority contexts override input mappings of lower priority contexts.

```cpp
auto gameContext = std::make_shared<TE::InputMappingContext>();
gameContext->Name = "Gameplay";
gameContext->AddMapping(jumpAction, TE::KeyCode::Space);

TE::InputSystem::Get().AddMappingContext(gameContext, 1);
```

#### `InputComponent::BindAction(action, callback)`
- **When Used**: Call on game entities during initialization (`OnAttach`) to bind gameplay functions to actions.

```cpp
auto inputComp = entity.AddComponent<TE::InputComponent>();
inputComp->BindAction(jumpAction, [](const TE::InputActionValue& val) {
    if (val.GetDigital()) {
        // Perform Jump logic
    }
});
```

#### `InputSystem::Get().Update(deltaTime)`
- **When Used**: Invoked every frame by `Application::Run()` to evaluate active contexts and trigger bound entity callbacks.
- **When NOT to use**: Do NOT invoke `Update()` manually inside game scripts.

---

## Related Architectural Documentation

- [Window Architecture](../Window/ARCHITECTURE.md) — OS window creation, GLFW callbacks, and input event hooks.
- [Scene & ECS Architecture](../Core/Scene/ARCHITECTURE.md) — Entity-Component-System (ECS) architecture for attaching `InputComponent` to entities.
- [Layers Subsystem Architecture](../Core/Layers/ARCHITECTURE.md) — Top-to-bottom event propagation and input consumption.


