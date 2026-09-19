# MaterialSystemPlugin Architecture

`MaterialSystemPlugin` provides a modular, graph-based node shader system and physical slab layering architecture for TimeEngine.

---

## 🏛️ System Overview

The material pipeline is separated into three primary tiers:

1. **Node Graph Data Model (`src/Graph/`)**: Built on TimeEngine's shared Core Graph Engine (`Engine/Include/Core/Graph/`), material nodes represent inputs (Constants, Scalar/Vector/Texture Parameters, Time, UVs), mathematical operations, physical slabs (Surface, Coat, Emissive, Dissolve, Unlit), and combination operators (Vertical Layering, Horizontal Blending, Additive Mixing).
2. **Optimized Shader Compiler (`src/Compiler/`)**: Traverses the DAG in topological order, eliminates redundant computations, and generates optimized single-pass GLSL shaders tailored for 2D forward rendering and dynamic 2D lighting passes.
3. **Asset & Instance Ecosystem (`src/Asset/` & `src/Editor/`)**: Exposes `.tematerial` graph assets and `.tematinst` material instance assets (with parameter overrides), alongside a real-time node canvas editor and preview viewport.

---

## 🔌 Plugin Integration Points

- **`OnLoad()`**:
  - Registers `MaterialAsset` (`.tematerial`) and `MaterialInstanceAsset` (`.tematinst`) with `AssetManager`.
  - Registers `MaterialAssetEditor` and `MaterialInstanceEditor` with `AssetEditorRegistry`.
  - Initializes node definitions in `MaterialNodeRegistry`.
- **`OnUnload()`**:
  - Unregisters asset types and editors gracefully.

---

## 💡 Conditional Features & Preprocessor Blocks

- **Lighting2D Plugin Integration**:
  - Under `#if defined(TE_PLUGIN_LIGHTING2D_ENABLED)`, the compiler and preview viewport hook into dynamic 2D normal mapping and multi-light accumulation passes.
- **MCP Plugin Automation**:
  - Under `#if defined(TE_PLUGIN_MCP_ENABLED)`, registers automated MCP tools for creating materials, adjusting parameters, and compiling shader graphs.
