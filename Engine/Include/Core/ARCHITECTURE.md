# Core Headers Subsystem Architecture Index (`Engine/Include/Core`)

This directory contains the public header interfaces, memory models, thread pools, and data structures for TimeEngine's core engine module.

---

## 🏛️ Subsystem Architecture Guides & Direct Links

* 🧵 **[Multi-Threading & Task System Architecture](file:///e:/TimeEngine/Engine/Include/Core/Threading/ARCHITECTURE.md)** (`Engine/Include/Core/Threading/`) — Dedicated worker thread pools (`ThreadPool`, `TaskSystem`), 6 task queues (Main, Render, Gameplay, AI, Calc, Widget), and async job submission macros (`SUBMIT_CALC`, `SUBMIT_AI`).
* ⚙️ **[Engine Settings Architecture](file:///e:/TimeEngine/Engine/Include/Core/EngineSettings_ARCHITECTURE.md)** (`Engine/Include/Core/EngineSettings_ARCHITECTURE.md`) — Singleton engine configuration (`EngineSettings`), target framerates, VSync, logging filters, performance caps, and `EngineSettingsLayer`.

---

## 🔗 Related Engine Source Architecture References

For detailed C++ implementation docs, refer to the corresponding source architecture references:

* ⚙️ **[Core Master Subsystem Architecture](file:///e:/TimeEngine/Engine/src/Core/ARCHITECTURE.md)** — Main engine loop (`Application`), event callbacks, and layer stack processing.
* 🎬 **[Scene & ECS Architecture](file:///e:/TimeEngine/Engine/src/Core/Scene/ARCHITECTURE.md)** — Entity-Component-System (`EntityManager`, `Entity`, `TComponent`).
* ⚛️ **[2D Physics Architecture](file:///e:/TimeEngine/Engine/src/Core/Physics/ARCHITECTURE.md)** — Rigid body dynamics (`RigidBody`) and 2D constraint joints.
* 💥 **[2D Collision Architecture](file:///e:/TimeEngine/Engine/src/Core/Collision/ARCHITECTURE.md)** — Broadphase pair filtering (`BroadPhase`) and SAT narrowphase overlap checks (`CollisionSystem`).
* 📦 **[Asset Subsystem Architecture](file:///e:/TimeEngine/Engine/src/Core/Asset/ARCHITECTURE.md)** — 64-bit `AssetHandle` mappings and path registry (`AssetRegistry`).
* 🎮 **[GameFramework Architecture](file:///e:/TimeEngine/Engine/src/Core/GameFrameWork/ARCHITECTURE.md)** — Base object reflection (`TObject`) and spatial components (`TComponent`).
* 🔌 **[Plugin Manager Architecture](file:///e:/TimeEngine/Engine/src/Core/Plugin/ARCHITECTURE.md)** — Dynamic OS library loader (`LoadLibraryW` / `dlopen`) and `.teplugin` parser.
