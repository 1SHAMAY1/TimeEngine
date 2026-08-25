# TimeEngine Out-of-the-Box Engine TScripts Library 📦

Welcome to the **TimeEngine Engine Content TScripts Library**. This directory contains modular, event-driven `.tscript` components designed to serve as high-level extensions, game mechanics, and behavioral coordinators that adhere to and extend TimeEngine's core C++ engine systems.

---

## 📁 Directory Structure & Script Catalog

```
Resources/Scripts/
├── Player/                # Extensions for PlayerMovementComponent2D & 2DPlayerController
│   ├── PlayerDashExtension.tscript
│   ├── PlayerStaminaExtension.tscript
│   ├── PlatformerDoubleJumpExtension.tscript
│   └── PlatformerWallMechanicsExtension.tscript
│
├── Camera/                # Extensions for PlayerCameraComponent & Camera Controllers
│   ├── CameraLookaheadExtension.tscript
│   ├── CameraBoundaryClamp.tscript
│   ├── CameraTraumaShakeExtension.tscript
│   └── CameraZoomController.tscript
│
├── Gameplay/              # Combat, Interaction, Trigger & Spawning Systems
│   ├── HealthDamageHandler.tscript
│   ├── ProjectileImpactFX.tscript
│   ├── TriggerEventDispatcher.tscript
│   ├── ProximityInteractable.tscript
│   └── WaveSpawner.tscript
│
├── AI/                    # Navigation & State Behaviors (NavigationAgent2D / StateTree)
│   ├── WaypointPatrolBehavior.tscript
│   └── AggroPursuitBehavior.tscript
│
├── Environment/           # World Props, Moving Platforms & Oscillators
│   ├── HarmonicFloatingProp.tscript
│   ├── OscillatingRotator.tscript
│   ├── WaypointMovingPlatform.tscript
│   └── DestructiblePropHandler.tscript
│
└── Utilities/             # Time Flow, Day/Night & Recurring Timer Dispatchers
    ├── TimedActionTrigger.tscript
    └── DayNightTimeManager.tscript
```

---

## 🎮 Script Details & Inspector Properties

### 1. Player Extensions (`/Player`)

| Script | Extends C++ Class | Key Registered Properties (`T_REGISTER_PROPERTY`) | Description |
| :--- | :--- | :--- | :--- |
| **`PlayerDashExtension.tscript`** | `PlayerMovementComponent2D` | `dash_speed` (650.0), `dash_duration` (0.18), `dash_cooldown` (1.0), `enable_invulnerability` (true) | High-speed dash impulse with cooldown handling and optional invulnerability frames. |
| **`PlayerStaminaExtension.tscript`** | `PlayerControllerBase` | `max_stamina` (100.0), `drain_rate` (25.0), `recovery_rate` (20.0), `recovery_delay` (1.2), `sprint_speed_multiplier` (1.6) | Stamina resource tracker managing sprint cost, delay before regeneration, and exhaustion penalty. |
| **`PlatformerDoubleJumpExtension.tscript`** | `PlayerMovementComponent2D` | `max_air_jumps` (1), `air_jump_multiplier` (0.9), `coyote_time` (0.15), `reset_on_ground` (true) | Multi-jump capability with configurable coyote-time buffer when running off ledges. |
| **`PlatformerWallMechanicsExtension.tscript`** | `PlayerMovementComponent2D` | `wall_slide_max_speed` (120.0), `wall_slide_friction` (8.0), `wall_jump_force_x` (320.0), `wall_jump_force_y` (450.0) | Wall-slide friction damping and directional wall-jump impulse coordinator. |

---

### 2. Camera Extensions (`/Camera`)

| Script | Extends C++ Class | Key Registered Properties (`T_REGISTER_PROPERTY`) | Description |
| :--- | :--- | :--- | :--- |
| **`CameraLookaheadExtension.tscript`** | `PlayerCameraComponent` | `lookahead_distance` (150.0), `return_speed` (4.0), `lead_speed` (6.0), `deadzone` (15.0) | Anticipates player movement direction by dynamically shifting camera offset with smooth damping. |
| **`CameraBoundaryClamp.tscript`** | `PlayerCameraComponent` | `min_bound_x` (-1000.0), `max_bound_x` (1000.0), `min_bound_y` (-600.0), `max_bound_y` (600.0) | Confines camera center within level rect bounding box limits. |
| **`CameraTraumaShakeExtension.tscript`** | `PlayerCameraComponent` | `trauma_decay_rate` (1.2), `max_shake_angle` (5.0), `max_shake_offset` (25.0), `frequency` (24.0) | Non-linear trauma-based camera shake generator with exponential decay. |
| **`CameraZoomController.tscript`** | `PlayerCameraComponent` | `default_size` (5.0), `max_zoom_out_size` (8.5), `zoom_speed` (3.5), `velocity_threshold` (200.0) | Dynamically zooms orthographic camera size when player exceeds velocity threshold. |

---

### 3. Gameplay & Combat (`/Gameplay`)

| Script | Extends C++ Class | Key Registered Properties (`T_REGISTER_PROPERTY`) | Description |
| :--- | :--- | :--- | :--- |
| **`HealthDamageHandler.tscript`** | `CollisionComponent` | `max_health` (100.0), `max_shield` (50.0), `armor_reduction` (0.15), `invulnerability_duration` (0.6) | Universal health/shield absorption system with armor damage reduction and i-frames. |
| **`ProjectileImpactFX.tscript`** | `ProjectileMovementComponent` | `direct_damage` (25.0), `splash_radius` (80.0), `splash_damage` (15.0), `max_ricochets` (0) | Impact trigger handler for splash radius damage, explosion effects, and ricochet counters. |
| **`TriggerEventDispatcher.tscript`** | `BoxColliderComponent` | `target_tag` ("Player"), `one_shot_only` (false), `retrigger_cooldown` (1.0) | Trigger overlap detector supporting tag filtering, cooldowns, and one-shot activations. |
| **`ProximityInteractable.tscript`** | `CollisionComponent` | `prompt_text` ("Press [E] to Interact"), `interaction_range` (120.0), `is_toggleable` (true) | Interaction prompt state machine with key input detection and toggle mode. |
| **`WaveSpawner.tscript`** | `GameManager` | `total_waves` (5), `base_spawns_per_wave` (6), `spawn_interval` (2.0), `time_between_waves` (10.0) | Wave spawner orchestrator with active enemy caps, wave progression, and rest intervals. |

---

### 4. AI & NPC Behaviors (`/AI`)

| Script | Extends C++ Class | Key Registered Properties (`T_REGISTER_PROPERTY`) | Description |
| :--- | :--- | :--- | :--- |
| **`WaypointPatrolBehavior.tscript`** | `NavigationAgent2DComponent` | `patrol_speed` (120.0), `waypoint_dwell_time` (1.5), `is_looping` (true), `total_waypoints` (4) | Waypoint patrol coordinator supporting PingPong and Loop modes with pause delays. |
| **`AggroPursuitBehavior.tscript`** | `StateTreeComponent` | `aggro_radius` (250.0), `attack_range` (45.0), `chase_speed` (180.0), `leash_distance` (450.0) | Proximity pursuit AI with detection radius, attack range, leash clamping, and return behavior. |

---

### 5. Environment & Props (`/Environment`)

| Script | Extends C++ Class | Key Registered Properties (`T_REGISTER_PROPERTY`) | Description |
| :--- | :--- | :--- | :--- |
| **`HarmonicFloatingProp.tscript`** | `TransformComponent` | `bob_amplitude` (15.0), `bob_frequency` (2.0), `phase_offset` (0.0), `animate_on_x` (false) | Smooth sinusoidal floating/bobbing object animation. |
| **`OscillatingRotator.tscript`** | `RotationComponent` | `min_angle` (-45.0), `max_angle` (45.0), `oscillation_speed` (60.0), `pause_at_extremes` (0.2) | Constrains and oscillates angular rotation between min/max angle boundaries. |
| **`WaypointMovingPlatform.tscript`** | `TransformComponent` | `travel_distance` (400.0), `move_speed` (80.0), `wait_time_at_ends` (1.0), `move_vertically` (false) | Ping-pong moving platform with endpoint dwell pauses. |
| **`DestructiblePropHandler.tscript`** | `CollisionComponent` | `durability` (50.0), `damage_stages` (3), `min_impact_force` (10.0), `spawn_loot_on_break` (true) | Breakable props with hit threshold, visual damage stages, and loot drop triggers. |

---

### 6. Utilities & Time Flow (`/Utilities`)

| Script | Extends C++ Class | Key Registered Properties (`T_REGISTER_PROPERTY`) | Description |
| :--- | :--- | :--- | :--- |
| **`TimedActionTrigger.tscript`** | `TimerComponent` | `initial_delay` (0.0), `repeat_interval` (1.0), `max_executions` (0), `timer_name` ("PeriodicEvent") | Multi-stage recurring/delayed event timer with execution limits. |
| **`DayNightTimeManager.tscript`** | `AmbientLightComponent` | `day_duration_seconds` (120.0), `time_scale` (1.0), `start_time_fraction` (0.25) | Deterministic day-night cycle progression simulator with Dawn/Day/Dusk/Night phase transitions. |

---

## 🛠️ How to Use in TimeEditor & Code

1. **Attach in Inspector**: Drag any `.tscript` file from the Content Browser into an Entity's Inspector, or add a `TScriptComponent` referencing the asset path (e.g. `Resources/Scripts/Player/PlayerDashExtension.tscript`).
2. **Configure Properties**: Modify any exposed `T_REGISTER_PROPERTY` field in the Inspector in real time without recompilation.
3. **Extend or Customize**: Open any `.tscript` file in the built-in TimeEditor Script Submode or your IDE to tweak parameters and event hooks.
