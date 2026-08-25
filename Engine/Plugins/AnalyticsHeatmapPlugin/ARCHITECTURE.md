# AnalyticsHeatmapPlugin Architecture

The `AnalyticsHeatmapPlugin` captures spatial player gameplay events (deaths, item pickups, failed jumps) and computes 2D Kernel Density Estimation (KDE) to render color-coded heatmaps over the **TimeEditor** viewport.

---

## 🏛️ System Architecture Flowchart

```mermaid
flowchart TD
    subgraph GameplaySession ["Runtime Telemetry Recorder"]
        Events["Gameplay Events (PlayerDeath, Stuck, JumpFail)"]
        Comp["AnalyticsRecorderComponent\n- Spatial Coordinates (x, y)\n- Timestamps & Event Types"]
        Events --> Comp
    end

    subgraph AnalyticsProcessing ["AnalyticsHeatmapPlugin (.dll / .teplugin)"]
        Exporter["Session Serializer (JSON/CSV Exporter)"]
        KDE["2D Kernel Density Estimation (KDE) Grid"]
        ColorMap["Color Gradient LUT (Blue -> Yellow -> Red)"]
        
        Comp --> Exporter
        Exporter --> KDE --> ColorMap
    end

    subgraph EditorViewport ["TimeEditor Viewport Overlay"]
        DrawList["TimeGUI / ImGui Viewport DrawList"]
        ColorMap --> DrawList
    end

    subgraph CrossPlugin ["Cross-Plugin Integrations"]
        MCP["#if defined(TE_HAS_PLUGIN_MCPPLUGIN)\nMCP Tools: Query Level Death Density Heatmap"]
        KDE --> MCP
    end
```

---

## 🛠️ Contributor Implementation Guide

- Implement `AnalyticsRecorderComponent::RecordEvent()` to push timestamped points into a ring buffer.
- Implement Gaussian 2D KDE in `src/KDE/KernelDensityEstimator.cpp`.
- Render heatmap quads using `TimeGUIDrawList::AddRectFilled()` with alpha blending.
