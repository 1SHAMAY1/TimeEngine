# TimeEngine Roadmap

The ultimate goal of TimeEngine is to provide a seamless, deterministic environment for **Time Manipulation** in 2D and 3D games.

## Phase 1: Foundation (Current)
- [x] High-performance OpenGL Renderer (2D/3D)
- [x] Entity-Component System (ECS)
- [x] YAML-based Serialization
- [x] Robust Editor UI with Docking
- [x] Project Hub and Asset Management
- [/] **Inbuilt 2D Sprite Editor**: Data-driven IDE with live procedural scripting (Nearly Complete).

## Phase 2: Determinism & State Management (Short-term)
- [ ] **Deterministic Core**: Ensure all engine systems (physics, logic) are deterministic across frames.
- [ ] **State Snapshoting**: Efficient system for capturing and storing entity states at specific time-steps.
- [ ] **Fixed Timestep**: Implementation of a strict fixed-update loop for logic.
- [ ] **Component Delta Compression**: Optimize state storage for large scenes.

## Phase 3: Time Manipulation APIs (Mid-term)
- [ ] **Time Reversal**: API to "rewind" the simulation state by traversing snapshots.
- [ ] **Pause & Step**: Granular control over simulation time (slo-mo, frame-by-frame).
- [ ] **Ghosting**: Visualize past/future states of entities simultaneously.
- [ ] **Non-Linear Time**: Support for branched timelines or localized time-dilation fields.

## Phase 4: Platform & Ecosystem (Long-term)
- [ ] Vulkan/DirectX 12 Backend
- [ ] C# or Lua Scripting Integration
- [ ] Physics Engine Integration (Box2D/PhysX) with deterministic wrappers.
- [ ] Multi-platform support (Linux, macOS).

---

Want to help? Check out [CONTRIBUTING.md](CONTRIBUTING.md) to get started!
