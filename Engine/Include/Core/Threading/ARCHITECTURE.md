# Multi-Threading & Task System Architecture

The Multi-Threading subsystem in TimeEngine provides dedicated thread pools ([`ThreadPool`](file:///e:/TimeEngine/Engine/Include/Core/Threading/ThreadPool.hpp)), task-type queue scheduling ([`TaskSystem`](file:///e:/TimeEngine/Engine/Include/Core/Threading/TaskSystem.hpp)), and submission macros ([`ThreadingMacros.hpp`](file:///e:/TimeEngine/Engine/Include/Core/Threading/ThreadingMacros.hpp)).

> [!NOTE]
> In short, think of the **Multi-Threading Subsystem** as the engine's multi-core task dispatcher: instead of running heavy computations on the main thread, `TaskSystem` maintains 6 specialized thread pools (Main, Render, Gameplay, AI, Calculation, Widget) and provides macro helpers (`SUBMIT_CALC`, `SUBMIT_AI`, `SUBMIT_RENDER`) to execute asynchronous jobs safely across CPU cores.

---

## Threading Architecture & Dedicated Pools

```
[ Application::Application() ]
              │
              ▼ (Initialize Worker Thread Pools)
┌─────────────────────────────────────────────────────────────┐
│                       TaskSystem                            │
├─────────────┬─────────────┬─────────────┬─────────────┬─────┴───────┐
│     MAIN    │   RENDER    │  GAMEPLAY   │     AI      │    CALC     │  WIDGET
│ ThreadPool  │ ThreadPool  │ ThreadPool  │ ThreadPool  │ ThreadPool  │ ThreadPool
└──────┬──────┴──────┬──────┴──────┬──────┴──────┬──────┴──────┬──────┴─────┬──┘
       │             │             │             │             │            │
       ▼             ▼             ▼             ▼             ▼            ▼
[ Main Loop ] [ GPU Submit ] [ Game Logic] [ Pathfinding] [ Physics/Math] [ ImGui UI ]
```

---

## Core Classes & Component Roles

1. **[`TaskSystem`](file:///e:/TimeEngine/Engine/Include/Core/Threading/TaskSystem.hpp)**:
   - Central static job dispatcher mapping `TaskType` enums (`MAIN`, `RENDER`, `GAMEPLAY`, `AI`, `CALC`, `WIDGET`) to dedicated `ThreadPool` instances.
   - Manages thread pool state toggles (`SetThreadEnabled`) and thread restarts (`RestartThread`).

2. **[`ThreadPool`](file:///e:/TimeEngine/Engine/Include/Core/Threading/ThreadPool.hpp)**:
   - Worker thread container spawning `std::thread::hardware_concurrency()` threads.
   - Uses `std::mutex`, `std::condition_variable`, and `std::queue<std::function<void()>>` for thread-safe work-stealing job queues.

3. **[`ThreadingMacros.hpp`](file:///e:/TimeEngine/Engine/Include/Core/Threading/ThreadingMacros.hpp)**:
   - Ergonomic macro suite for initializing pools and submitting asynchronous jobs.

---

## Submission Macros & Usage Guidelines

### 1. Engine Initialization (`INIT_*`)
Called automatically inside `Application::Application()`:
- `INIT_MAIN_THREAD()`
- `INIT_RENDER_THREAD()`
- `INIT_GAMEPLAY_THREAD()`
- `INIT_AI_THREAD()`
- `INIT_CALC_THREAD()`
- `INIT_WIDGET_THREAD()`

---

### 2. Submitting Asynchronous Jobs

#### `SUBMIT_CALC(job)`
- **When Used**: Offload heavy mathematical calculations, procedural mesh generation, or physics sub-stepping to worker threads.

```cpp
SUBMIT_CALC([this]() {
    // Heavy math calculation on background CALC thread
    CalculateComplexNoise();
});
```

#### `SUBMIT_AI(job)`
- **When Used**: Run AI pathfinding algorithms (A* search, navigation mesh queries) or behavior tree evaluations off the main thread.

```cpp
SUBMIT_AI([npcEntity]() {
    // Asynchronous A* pathfinding evaluation
    npcEntity->ComputePathToPlayer();
});
```

#### `SUBMIT_RENDER(job)`
- **When Used**: Enqueue asynchronous texture decoding or material pipeline compilation tasks.

---

## Related Architectural Documentation

- [Core Subsystem Architecture](../../../src/Core/ARCHITECTURE.md) — Main engine application loop.
- [2D Physics Architecture](../../../src/Core/Physics/ARCHITECTURE.md) — Asynchronous physics integration.
- [Root Architecture Index](../../../../ARCHITECTURE.md) — Master TimeEngine architecture hub.

