# 2D IKPlugin Architecture

The `IKPlugin` provides high-performance, purely mathematical 2D Inverse Kinematics (IK) solvers, constraint systems, and procedural foot grounding adaptation for TimeEngine.

---

## 🏛️ Ecosystem Overview

```mermaid
flowchart TD
    subgraph Solvers ["2D Solvers"]
        TwoBone["TwoBoneIKSolver2D (2D Limbs)"]
        FABRIK["FABRIKSolver2D (2D Multi-Joint Chains)"]
        CCD["CCDIKSolver2D (2D Constrained Joints)"]
        Aim["AimConstraint2D (2D Head/Weapon Targeting)"]
        Ground["FootGrounder2D (2D Terrain Grounding)"]
    end

    subgraph ECS ["ECS Layer"]
        Chain["IKChain2DComponent"] --> TwoBone
        Chain --> FABRIK
        Target["IKTarget2DComponent"] --> Chain
    end

    subgraph Gameplay ["Gameplay & AI"]
        Lib["IKGameplayLib (TFunctionLibrary)"] --> Chain
        MCP["IKMCPTools"] --> Solvers
    end
```

---

## 🔑 Key Subsystems

1. **`TwoBoneIKSolver2D`**: Exact analytical law-of-cosines 2-joint limb solver for character knees, elbows, and arms in 2D.
2. **`FABRIKSolver2D`**: Forward And Backward Reaching Inverse Kinematics for arbitrary length 2D chains (tails, spines, ropes, tentacles).
3. **`CCDIKSolver2D`**: Cyclic Coordinate Descent solver with per-joint angular clamping in 2D.
4. **`AimConstraint2D`**: Look-at solver directing 2D heads, eyes, and weapons toward target mouse / world coordinates.
5. **`FootGrounder2D`**: Procedural foot placement and pelvis adjustment based on raycast ground contact in 2D.
6. **`IKGameplayLib`**: Stateless gameplay helpers deriving from `TFunctionLibrary`.
7. **`IKMCPTools`**: Preprocessor-guarded AI automation tools for posing and 2D IK solver execution.
