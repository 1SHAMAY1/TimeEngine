# Scene Subsystem Architecture

The Scene subsystem in TimeEngine provides the Entity-Component-System (ECS) architecture, entity lifespan management ([`EntityManager`](file:///e:/TimeEngine/Engine/Include/Core/Scene/EntityManager.hpp)), scene graph lifecycle ([`Scene`](file:///e:/TimeEngine/Engine/Include/Core/Scene/Scene.hpp)), component reflection ([`ComponentRegistry`](file:///e:/TimeEngine/Engine/Include/Core/Scene/ComponentRegistry.hpp)), and JSON scene serialization.

> [!NOTE]
> In short, think of the **Scene Subsystem** as the movie stage: `Scene` represents the stage holding all active props and actors; `Entity` represents an individual actor ID on stage; and `TComponent` (like `SpriteComponent`, `TransformComponent`, `PlayerMovementComponent2D`) are the costumes, scripts, and abilities attached to each actor.

---

## Core Component Overview & Hierarchy

```
                          [ Scene ] (.tescene Asset)
                             │
                             ▼
                    [ EntityManager ]
                             │
       ┌─────────────────────┼─────────────────────┐
       ▼                     ▼                     ▼
[ Entity (ID 1) ]     [ Entity (ID 2) ]     [ Entity (ID 3) ]
  ├─ TagComponent       ├─ TagComponent       ├─ TagComponent
  ├─ TransformComponent ├─ TransformComponent ├─ TransformComponent
  └─ SpriteComponent    └─ BoxComponent       └─ PlayerMovementComponent2D
```

---

## Subsystem Classes & Usage Guidelines

### 1. `TE::Scene` (Scene Graph & Asset Container)

The `Scene` class inherits from `Asset` and manages scene execution states:

#### `Scene::CreateEntity(name)` / `Scene::DestroyEntity(entity)`
- **When Used**: Call when spawning new game objects (bullets, enemies, items) or destroying them during gameplay.
- **What it does**: Allocates a unique `EntityID` in `EntityManager` and automatically attaches a `TagComponent` and `TransformComponent`.

```cpp
// Create a named game object inside the active scene
Entity player = m_ActiveScene->CreateEntity("Player");
```

#### `Scene::OnRuntimeStart()` & `Scene::OnRuntimeStop()`
- **When Used**: Invoked by `EditorLayer` when toggling Play / Stop modes in the editor toolbar. Initializes physics simulation (`PhysicsWorld`) and game component runtime state.

#### `Scene::OnUpdateRuntime(float dt)`
- **When Used**: Invoked every frame during active game playback to update physics, movement components, and script controllers.

---

### 2. `TE::EntityManager` & `TE::Entity` (ECS Container)

#### `Entity::AddComponent<T>(args...)`
- **When Used**: Attach a component to an entity (e.g., adding `SpriteComponent` or 2D physics `BoxComponent`).
- **Rule**: Component type `T` MUST derive from `TComponent`.

```cpp
// Attach 2D transform and sprite renderer to entity
auto* spriteComp = entity.AddComponent<TE::SpriteComponent>();
```

#### `Entity::GetComponent<T>()` / `Entity::HasComponent<T>()`
- **When Used**: Retrieve a pointer to an attached component or check if an entity possesses a specific capability.
- **When NOT to use**: Do NOT cache component raw pointers across multiple frames if the component or entity might be deleted.

```cpp
if (entity.HasComponent<TE::TransformComponent>()) {
    auto* transform = entity.GetComponent<TE::TransformComponent>();
    transform->Position.x += 5.0f * dt;
}
```

#### `EntityManager::GetAliveEntities()`
- **When Used**: Called by renderers and editor panels (like Scene Hierarchy Panel) to iterate through all active entity IDs in the scene.

---

### 3. Core Engine Components

- **`TagComponent`**: Holds entity display name string.
- **`TransformComponent`**: Stores 2D/3D `Position`, `Rotation`, and `Scale`.
- **`SpriteComponent`**: Holds texture handle and UV coordinates for 2D rendering.
- **`PlayerMovementComponent2D`**: 2D movement logic with speed and acceleration.
- **`BoxComponent` / `CircleComponent`**: 2D collision geometry components.
