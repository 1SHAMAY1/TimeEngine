# Layers & LayerStack Architecture

The Layers subsystem in TimeEngine handles modular execution stacks ([`LayerStack`](file:///e:/TimeEngine/Engine/Include/Layers/LayerStack.hpp)), application update dispatching, GUI rendering passes, and event propagation pipelines ([`Layer`](file:///e:/TimeEngine/Engine/Include/Layers/Layer.hpp)).

> [!NOTE]
> In short, think of the **Layers Subsystem** as a stack of transparent animation sheets: regular game layers (like 2D Scene, Camera, or Game World) sit at the bottom, while overlay layers (like `TimeGUILayer`, Editor Panels, Profiler, and Settings Dialogs) sit on top. Every frame, the engine updates and draws layers from back to front, but passes user events from top to bottom so overlays can consume inputs first.

---

## Layer Pipeline & Data Flow

```
[ OS Hardware Event ]
         │
         ▼ (Propagated TOP-TO-BOTTOM)
┌────────────────────────────────────────┐
│ Overlays (TimeGUILayer, Profiler, etc) │ ──► Can mark event as Handled!
├────────────────────────────────────────┤
│ Application Layers (EditorLayer, etc) │
└────────────────────────────────────────┘
         │
         ▼ (Rendered BOTTOM-TO-TOP)
[ Final Viewport Frame Screen ]
```

---

## Core Classes & Subsystem Roles

1. **[`TE::Layer`](file:///e:/TimeEngine/Engine/Include/Layers/Layer.hpp)**: Base class defining life cycle hooks:
   - `OnAttach()`: Called when layer is pushed to `LayerStack`.
   - `OnDetach()`: Called when layer is popped or removed.
   - `OnUpdate()`: Frame logic and rendering pass.
   - `OnTimeGUIRender()`: Immediate-mode GUI rendering pass.
   - `OnEvent(event)`: Input event handler.
2. **[`TE::LayerStack`](file:///e:/TimeEngine/Engine/Include/Layers/LayerStack.hpp)**: Vector container dividing normal layers (`0` to `m_LayerInsertIndex`) from overlays (`m_LayerInsertIndex` to end).

---

## Key Functions & Usage Guidelines

### 1. `LayerStack::PushLayer(layer)` vs `LayerStack::PushOverlay(overlay)`
- **`PushLayer(layer)`**: Pushes a standard application layer (e.g. `EditorLayer`, `CameraLayer`). Inserted in the lower half of the stack before overlays.
- **`PushOverlay(overlay)`**: Pushes a high-priority overlay layer (e.g. `TimeGUILayer`, `ProfilingLayer`, `EngineSettingsLayer`). Inserted at the top of the stack so it receives events first and draws over game content.

```cpp
// Pushing standard layer vs top-level overlay
m_LayerStack.PushLayer(new EditorLayer());
m_LayerStack.PushOverlay(new TimeGUILayer());
```

---

### 2. Deferred Layer Removal (`MarkLayerForRemoval` & `ProcessDeferredRemovals`)
- **When Used**: Called when closing modal overlay layers (like `EngineSettingsLayer` or `ProjectHubLayer`).
- **Why Deferred**: Prevents iterator invalidation and crashes if a layer requests its own removal while `LayerStack` is iterating through `OnUpdate()` or `OnEvent()`.

```cpp
// Mark layer for safe removal at end of frame
m_LayerStack.MarkLayerForRemoval(this);
```

---

### 3. Layer Hooks

#### `Layer::OnEvent(event)`
- **When Used**: Called during top-to-bottom event dispatching in `Application::OnEvent()`.
- **Rule**: If an overlay consumes an event (e.g. clicking a GUI button on `TimeGUILayer`), set `event.Handled = true` to stop the event from falling through to game viewport layers below.

```cpp
void EditorLayer::OnEvent(TE::Event &e) {
    m_Camera.OnEvent(e);
    if (e.Handled) return; // Stopped if consumed
}
```

---

## Active Engine Layers

- **`EditorLayer`**: Main 2D editor workspace layer.
- **`TimeGUILayer`**: Core ImGui context layer.
- **`ProjectHubLayer`**: Startup project browser dialog.
- **`ProfilingLayer`**: Real-time FPS, CPU, and GPU performance overlay.
- **`EngineSettingsLayer`**: Engine configuration and graphics backend options.
- **`CameraLayer`**: Viewport camera management.

---

## Related Architectural Documentation

- [Editor Architecture](file:///e:/TimeEngine/Engine/src/Editor/ARCHITECTURE.md) — Documentation for `EditorLayer` and editor modes.
- [Window Architecture](file:///e:/TimeEngine/Engine/src/Window/ARCHITECTURE.md) — Documentation for OS window creation and event callback dispatch.
- [Utils & TimeGUI Architecture](file:///e:/TimeEngine/Engine/src/Utils/ARCHITECTURE.md) — Documentation for `TimeGUI` rendering primitives.
