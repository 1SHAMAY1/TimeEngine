# GameFramework Subsystem Architecture

The GameFramework subsystem in TimeEngine provides Unreal-inspired core object reflections ([`TObject`](file:///e:/TimeEngine/Engine/Include/GameFrameWork/TObject.hpp)), component behaviors ([`TComponent`](file:///e:/TimeEngine/Engine/Include/GameFrameWork/TComponent.hpp)), helper macro libraries ([`TFunctionLibrary`](file:///e:/TimeEngine/Engine/Include/GameFrameWork/TFunctionLibrary.hpp)), and global object lifecycle pooling ([`StandardGameLibrary`](file:///e:/TimeEngine/Engine/Include/GameFrameWork/TStandardGameLibrary.hpp)).

> [!NOTE]
> In short, think of the **GameFramework Subsystem** as the base foundation for all game objects: `TObject` gives every object a unique ID, owner, and initialization flag; `TComponent` provides spatial 2D transforms, hierarchy parent/child relationships, custom geometry picking, and 2D shadow occlusion features; while `StandardGameLibrary` provides object pooling and instantiation helpers (`CreateObject`).

---

## Subsystem Architecture & Data Flow

```
                     [ TObject ]  (Base Object with ID, Name, Owner)
                          │
                          ▼
                    [ TComponent ]  (ECS Component Base)
                          │
       ┌──────────────────┼──────────────────┐
       ▼                  ▼                  ▼
[ Transform ]    [ Geometry & Shadows ] [ Inspector GUI ]
(TETransform)    (ContainsPoint,        (OnDrawInspector)
                  GetWorldVertices)
```

---

## Core Classes & Subsystem Roles

1. **[`TObject`](file:///e:/TimeEngine/Engine/Include/GameFrameWork/TObject.hpp)**:
   - Root object class for all reflected game objects.
   - Assigns thread-safe unique 64-bit IDs via `std::atomic<uint64_t>`.
   - Tracks object names, owner relationships (`SetOwner`), and pending destruction states (`MarkPendingDestroy`).

2. **[`TComponent`](file:///e:/TimeEngine/Engine/Include/GameFrameWork/TComponent.hpp)**:
   - Abstract base class for all ECS components in TimeEngine (inherits object ownership from `TObject`).
   - Bundles spatial 2D transformation (`TETransform Transform`), component hierarchy (`SetComponentParent`, `Children`), picking (`ContainsPoint`), 2D shadow occlusion (`CastsOcclusionShadow`), and inspector drawing (`OnDrawInspector`).

3. **[`TFunctionLibrary`](file:///e:/TimeEngine/Engine/Include/GameFrameWork/TFunctionLibrary.hpp)** & **[`StandardGameLibrary`](file:///e:/TimeEngine/Engine/Include/GameFrameWork/TStandardGameLibrary.hpp)**:
   - Provides static game utility functions and central `TObject` heap pooling (`g_ObjectPool`).

---

## Core Functions & Usage Guidelines

### 1. `TObject` Core Functions

#### `TObject::GetID()` & `TObject::SetOwner(newOwner)`
- **When Used**: Call to check unique runtime object IDs or establish owner-child relationships between game objects.

#### `TObject::MarkPendingDestroy()`
- **When Used**: Mark objects for deferred cleanup at the end of the frame step.
- **When NOT to use**: Do NOT manually `delete` raw `TObject*` pointers directly if they are owned by `EntityManager` or `StandardGameLibrary`.

---

### 2. `TComponent` Core Functions

#### `TComponent::OnInitialize()`, `OnAttach()`, `OnDetach()`
- **`OnInitialize()`**: Called when component memory is allocated.
- **`OnAttach()`**: Invoked when attached to an active `Entity`. Use for setting up physics bodies, input bindings, or rendering buffers.
- **`OnDetach()`**: Invoked when removed from an entity. Use for cleanup.

#### `TComponent::ContainsPoint(worldModel, point)` & `GetWorldVertices(worldModel)`
- **When Used**: Override in 2D renderable components (`BoxComponent`, `SpriteComponent`, `CircleComponent`) to support mouse selection picking in the editor viewport or 2D shadow casting calculations.

```cpp
// Override in custom 2D component for editor mouse picking
virtual bool ContainsPoint(const TE::TEMatrix4 &worldModel, const TE::TEVector2 &point) const override {
    // Return true if point lies inside component bounding box
    return false;
}
```

#### `TComponent::SetComponentParent(newParent)`
- **When Used**: Build nested component hierarchies (e.g., attaching a weapon component to a character arm component). Automatically manages parent/child lists.

---

### 3. `StandardGameLibrary` Object Creation

#### `StandardGameLibrary::CreateObject<T>(args...)`
- **When Used**: Instantiates standalone `TObject`-derived instances into the global object pool (`g_ObjectPool`).

```cpp
auto& myObj = StandardGameLibrary::CreateObject<MyCustomObject>("GameManager");
```

---

## Related Architectural Documentation

- [Scene & ECS Architecture](file:///e:/TimeEngine/Engine/src/Core/Scene/ARCHITECTURE.md) — Documentation for `EntityManager` and component lifecycle.
- [Utils & Math Architecture](file:///e:/TimeEngine/Engine/src/Utils/ARCHITECTURE.md) — Documentation for `TETransform` and 2D vector math primitives.
