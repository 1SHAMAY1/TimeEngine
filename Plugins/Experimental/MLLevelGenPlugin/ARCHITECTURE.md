# MLLevelGenPlugin Architecture

The `MLLevelGenPlugin` integrates machine learning algorithms (such as Wave Function Collapse pattern synthesis and neural level generator models) into TimeEngine's procedural generation ecosystem.

---

## 🏛️ System Architecture Flowchart

```mermaid
flowchart TD
    subgraph PCGNodeGraph ["PCGPlugin Visual Graph Editor"]
        WFCNode["PCGMLWFCNode\n(Learned Pattern Model)"]
        GANNode["PCGGANRoomNode\n(Neural Room Generator)"]
    end

    subgraph MLLevelGenCore ["MLLevelGenPlugin (.dll / .teplugin)"]
        WFCPropagator["WFC Constraint Solver\n- Tile Adjacency Frequency Matrices\n- Entropy Minimization"]
        ModelInference["Tiny / ONNX Grid Inference\n(Generates Tile ID matrices)"]
        
        WFCNode --> WFCPropagator
        GANNode --> ModelInference
    end

    subgraph EngineIntegrations ["Engine & Inter-Plugin Integrations"]
        ThreadPool["SUBMIT_AI(job)\n(ThreadingMacros.hpp)"]
        Tilemap["TilemapComponent\n(Applies Generated 2D Grid)"]
        Tags["#if defined(TE_HAS_PLUGIN_GAMEPLAYTAGPLUGIN)\nBiome & Difficulty Tag Seeding"]
        
        WFCPropagator --> ThreadPool
        ModelInference --> ThreadPool
        ThreadPool --> Tilemap
        WFCPropagator --> Tags
    end
```

---

## 🛠️ Contributor Implementation Guide

- Implement constraint propagation in `src/Nodes/PCGMLWFCNode.cpp`.
- Connect `#if defined(TE_HAS_PLUGIN_PCGPLUGIN)` to register nodes into the PCG node palette.
- Run tilemap generation tasks on `SUBMIT_AI` background worker threads.
