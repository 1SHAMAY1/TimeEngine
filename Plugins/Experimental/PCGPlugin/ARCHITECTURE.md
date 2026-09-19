# PCGPlugin Architecture

The `PCGPlugin` provides a high-performance 2D/3D Procedural Content Generation (PCG) toolset, node execution graph engine, ECS volume component, and visual asset editor for TimeEngine.

---

## 🏛️ System Architecture Flowchart

```mermaid
flowchart TD
    subgraph EditorWorkspace ["TimeEditor Workspace"]
        AssetBrowser["ContentBrowserPanel\n(Create / Open .tepcg)"]
        AssetEditor["PCGGraphAssetEditor : AssetEditor\n- Uses Core NodeCanvas\n- Uses Core NodePalettePopup\n- Parameter Inspector Drawer"]
        ViewportGizmo["PCGVolumeComponent Drawer\n(Seed, Bounds, Auto-Regen on Edit)"]
        
        AssetBrowser --> AssetEditor
    end

    subgraph CorePlugin ["PCGPlugin Core Engine"]
        PluginEntry["PCGPlugin (.dll / .teplugin)"]
        Asset["PCGGraphAsset (.tepcg)"]
        Graph["PCGGraph : Core Graph\n(Uses Core GraphNode, GraphPin, GraphConnection)"]
        Context["PCGContext\n- Spatial Bounds\n- Seeded Random Stream (MathUtils)\n- PCGPointData Collections"]
        
        subgraph NodeEcosystem ["PCG Node Library"]
            Generators["Point Generators\n(Grid, Random Scatter, Poisson Disk, Spline/Path)"]
            TerrainNoise["Noise & Terrain Nodes\n(Perlin, Simplex, Worley, Elevation, Slope)"]
            Structures["Structural Creators\n(Wilson's Maze, Cellular Automata, BSP, Drunkard Walk, WFC)"]
            Processors["12 Specialized ECS Processors\n(Voronoi, Delaunay, XPBD Relaxer, Raycast Snapping,\nBounds Exclusion, Attribute Injector, Hierarchy Linker,\nL-System, Edge Extractor, K-Means Clustering, A* Traversal, Proximity Mutator)"]
            Spawners["Entity & Tile Spawners\n(EntitySpawnerNode, TilemapSpawnerNode, PrefabAssembler)"]
        end
    end

    subgraph EngineIntegrations ["TimeEngine Core Integrations"]
        ECS["ECS Scene & Components\n(PCGVolumeComponent, Entity, TransformComponent)"]
        Physics["PhysicsWorld & Velox\n(XPBD Raycast Snapping & Collision Filtering)"]
        Renderer["Renderer2D\n(Debug Point Cloud & Shape Batch Rendering)"]
        MCP["MCP Automation Tools\n(#if defined(TE_HAS_PLUGIN_MCPPLUGIN))"]
    end

    AssetEditor --> Graph
    Graph --> Context
    Context --> Generators & TerrainNoise & Structures & Processors & Spawners
    Spawners --> ECS
    Processors --> Physics
    ViewportGizmo --> Renderer
    MCP --> Asset
```

---

## 🔑 Key Features & Subsystems

1. **Leverages TimeEngine Core Graph**:
   - Extends native `Graph`, `GraphNode`, `GraphPin`, and `GraphConnection`.
   - Uses `NodeCanvas` and `NodePalettePopup` for full visual authoring, zoom/pan, and cable routing.

2. **Rich Procedural Node Suite**:
   - **Point Generators**: Grid, Random Scatter, Poisson Disk (Bridson's algorithm), and Spline Paths.
   - **Noise & Terrain**: Perlin, Simplex, Worley (Cellular), and Slope/Elevation map filters.
   - **Structural Solvers**: Wilson's uniform spanning tree maze, Cellular Automata cave carver, BSP rooms, Drunkard's Walk, and 2D Wave Function Collapse (WFC).
   - **12 Specialized ECS Processors**: Voronoi partitioning, Delaunay triangulation, XPBD physics point relaxation, Raycast collision snapping, Bounds exclusion masking, ECS attribute injection, Prefab hierarchy linking, L-System branching, Perimeter edge extraction, K-Means density clustering, A* pathfinding traversal, and Component proximity mutation.

3. **ECS Volume Component (`PCGVolumeComponent`)**:
   - Attachable to any entity in the scene to drive procedural generation with custom volume bounds, seeds, and runtime/editor regeneration triggers.

4. **Self-Contained MCP Tools**:
   - Automated AI graph generation and execution over the Model Context Protocol bridge.
