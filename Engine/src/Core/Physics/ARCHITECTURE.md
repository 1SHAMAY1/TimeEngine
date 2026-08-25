# 2D Physics Subsystem Architecture

The 2D Physics subsystem in TimeEngine handles rigid body dynamics ([`RigidBody`](../../../../Include/Core/Physics/PhysicsWorld.hpp)), symplectic Euler numerical integration, 2D raycasting, joint constraints (Distance, Revolute, Prismatic, Gear, Pulley), soft body simulation, and ECS integration ([`RigidBodyComponent`](../../../../Include/Core/Physics/RigidBodyComponent.hpp)).

> [!NOTE]
> In short, think of the **Physics Subsystem** as the game's gravity and collision simulation engine: `PhysicsWorld` steps simulation time, calculates forces and accelerations, resolves collisions, and applies gravity to all attached `RigidBody` components.

---

## Core Component Overview

1. **[`TE::RigidBody`](../../../../Include/Core/Physics/PhysicsWorld.hpp)**: Structure storing 2D physical properties (`Position`, `Velocity`, `Force`, `Mass`, `Restitution`, `IsStatic`).
2. **[`TE::PhysicsWorld`](../../../../Include/Core/Physics/PhysicsWorld.hpp)**: Physics scene simulation container handling raycasts, gravity, collision resolution, joints, and soft bodies.
3. **[`TE::RigidBodyComponent`](../../../../Include/Core/Physics/RigidBodyComponent.hpp)**: ECS component wrapping a `RigidBody` instance onto scene entities.

---

## Core Functions & Usage Guidelines

### 1. `RigidBody::ApplyForce(force)` & `Integrate(dt)`
- **`ApplyForce(force)`**: Accumulates linear forces (impulses, wind, thrusters).
- **`Integrate(dt)`**: Uses Symplectic Euler integration to update velocity and position:
  $$\vec{a} = \vec{F} \cdot m^{-1}, \quad \vec{v}_{t+1} = \vec{v}_t + \vec{a} \cdot \Delta t, \quad \vec{p}_{t+1} = \vec{p}_t + \vec{v}_{t+1} \cdot \Delta t$$

```cpp
// Apply upward jump force to a rigid body component
auto* rb = entity.GetComponent<TE::RigidBodyComponent>();
rb->AddForce(TEVector2(0.0f, 500.0f));
```

---

### 2. `PhysicsWorld::Step(dt)`
- **When Used**: Called every fixed frame (or sub-step) by `Scene::OnUpdateRuntime(dt)` to advance physics simulation, integrate forces, and resolve collision overlaps.
- **When NOT to use**: Do NOT call `Step()` inside GUI render passes or frame rendering loops.

---

### 3. `PhysicsWorld::Raycast(start, direction, maxDistance, ...)`
- **When Used**: Call for line-of-sight checks, bullet hit detection, or ground detection in player controllers.

```cpp
TEVector2 hitPoint, hitNormal;
float fraction;
uint32_t hitEntityID;

bool hit = physicsWorld->Raycast(
    playerPos, TEVector2(0.0f, -1.0f), 1.5f,
    hitPoint, hitNormal, fraction, hitEntityID
);

if (hit) {
    // Player is grounded!
}
```

---

### 4. 2D Joint System
- **`AddDistanceJoint(...)`**: Keeps two bodies at a fixed distance (ropes, springs).
- **`AddRevoluteJoint(...)`**: Creates a 2D pin/hinge joint with optional motor torque and angle limits.
- **`AddPrismaticJoint(...)`**: Constrains movement along a single 2D translation axis (pistons, sliders).
- **`AddGearJoint(...)` / `AddPulleyJoint(...)`**: Constrains rotational ratio or pulley cable lengths between entities.

---

### 5. Soft Body System
- **`CreateSoftBodyBlob(center, radius, nodeCount, ...)`**: Generates a deformable soft-body blob composed of interconnected spring-damper nodes.
- **`CreateSoftBodyShapeMatched(...)`**: Creates shape-matched soft body geometry from custom vertex arrays.
