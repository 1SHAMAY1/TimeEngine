# MotionMatchingPlugin Architecture

The `MotionMatchingPlugin` provides a unified **Motion Matching** animation synthesis engine designed for **2D Sprite Flipbooks**, **2D Deformable Skeletal Rigs**, and extensible to **3D Skeletal Animation**.

---

## 🏛️ System Architecture Flowchart

```mermaid
flowchart TD
    subgraph AssetLayer ["Animation Database Asset (.temotion)"]
        SourceAsset["Source Animation Asset\n(SpriteAtlas / SkeletalClip2D / 3D Animation)"]
        Extractor["Feature Vector Extractor\n- Velocity (vx, vy, vz)\n- Trajectory Prediction (0.2s, 0.4s, 0.8s)\n- Joint/Bone Positions & Phases"]
        SpatialIndex["Spatial KD-Tree / Ball-Tree Index"]
        
        SourceAsset --> Extractor --> SpatialIndex --> MotionDB["MotionMatchingDatabaseAsset"]
    end

    subgraph RuntimeECS ["Runtime ECS Components"]
        Entity["Character Entity"]
        Comp["MotionMatchingComponent\n(Mode: 2D Sprite / 2D Skeletal / 3D Rig)"]
        
        Entity --> Comp
    end

    subgraph MotionMatchingEngine ["Motion Matching Solver"]
        Query["Build Query Feature Vector"]
        Search["Nearest-Neighbor KD-Tree Search (< 0.005 ms)"]
        Blender["Inertialization & Pose Blender"]
        
        Comp --> Query --> Search
        MotionDB --> Search --> Blender --> Comp
    end

    subgraph CrossPlugin ["Cross-Plugin Integrations"]
        SpriteEditor["#if defined(TE_HAS_PLUGIN_SPRITEEDITORPLUGIN)\nSprite Studio Foot Phase Tagging"]
        Skeletal2D["#if defined(TE_HAS_PLUGIN_SKELETAL2DPLUGIN)\n2D Bone Trajectory Matching"]
        IK["#if defined(TE_HAS_PLUGIN_IKPLUGIN)\nFoot Placement & Ground Adaptation"]
        
        Blender --> SpriteEditor
        Blender --> Skeletal2D
        Blender --> IK
    end
```

---

## 📐 Unified Motion Matching Cost Function

The query cost function evaluates both the physical trajectory and joint/pose features across dimensions:
$$\text{Cost} = w_v \|\mathbf{v}_{\text{query}} - \mathbf{v}_{\text{frame}}\|^2 + w_t \sum_{i=1}^{K} \|\mathbf{p}_{\text{future}, i} - \mathbf{p}_{\text{frame}, i}\|^2 + w_\theta |\Delta \theta| + w_p (\text{PhaseMatch}) + w_j \sum_{j} \|\mathbf{x}_{\text{joint}, j}^{\text{query}} - \mathbf{x}_{\text{joint}, j}^{\text{frame}}\|^2$$

---

## 🛠️ Contributor Implementation Guide

- Extend `MotionMatchingComponent` to handle 2D sprite frames, 2D skeletal rigs, and future 3D meshes.
- Implement spatial search queries in `src/MotionMatchingDatabase.cpp`.
- Connect `#if defined(TE_HAS_PLUGIN_IKPLUGIN)` for post-search foot adjustment.
