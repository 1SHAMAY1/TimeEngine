# Renderer Subsystem Architecture

The Renderer subsystem in TimeEngine provides multi-API graphics abstractions (OpenGL, DirectX 11, OpenGLES, Vulkan), high-performance batch rendering ([`Renderer2D`](file:///e:/TimeEngine/Engine/src/Renderer/Renderer2D.cpp) & [`RenderBatcher`](file:///e:/TimeEngine/Engine/Include/Renderer/RenderBatcher.hpp)), 2D lighting & shadow casting, materials ([`Material`](file:///e:/TimeEngine/Engine/Include/Renderer/Material/Material.hpp)), shaders ([`ShaderLibrary`](file:///e:/TimeEngine/Engine/Include/Renderer/ShaderLibrary.hpp)), textures ([`Texture`](file:///e:/TimeEngine/Engine/Include/Renderer/Texture.hpp)), and framebuffers ([`Framebuffer`](file:///e:/TimeEngine/Engine/Include/Renderer/Framebuffer.hpp)).

> [!NOTE]
> In short, think of the **Renderer Subsystem** as the engine's master artist and GPU driver: `Renderer2D` batches thousands of 2D quads, triangles, circles, ambient lighting passes, and 2D shadow volume projections into single draw calls via `RenderBatcher`, while low-level classes (`VertexArray`, `VertexBuffer`, `IndexBuffer`, `RenderCommand`) abstract away raw backend OpenGL / DirectX calls.

---

## Subsystem Pipeline & Architecture

```
[ Application / Editor Layer ]
               │
               ▼ (Submit Geometry, Lights & Materials)
[ Renderer2D ]
               │
               ├─► SubmitQuad / SubmitTriangle / SubmitCircle / SubmitLine
               ├─► SubmitLight (Point, Spot, Line Lights)
               └─► SubmitShadow (Silhouette Ray Projection Shadow Volumes)
               │
               ▼ (Batch & Sort by Material / Blend Mode)
[ RenderBatcher ]
               │
               ▼ (Low-Level Execution)
[ RenderCommand ] ──► [ Graphics API: OpenGL / DX11 / Vulkan / OpenGLES ]
```

---

## Core Classes & Subsystem Roles

1. **[`TE::Renderer2D`](file:///e:/TimeEngine/Engine/src/Renderer/Renderer2D.cpp)**: High-level 2D rendering pipeline handling quads, circles, debug outlines, 2D lights, ambient sky/ground gradients, and 2D shadow volume projections.
2. **[`TE::RenderBatcher`](file:///e:/TimeEngine/Engine/Include/Renderer/RenderBatcher.hpp)**: Optimization engine sorting draw calls by material and texture handles to minimize GPU state changes.
3. **[`TE::Material`](file:///e:/TimeEngine/Engine/Include/Renderer/Material/Material.hpp)**: Shader uniform wrapper binding textures, colors, and material properties.
4. **[`TE::ShaderLibrary`](file:///e:/TimeEngine/Engine/Include/Renderer/ShaderLibrary.hpp)**: Factory for built-in engine shaders (`ColorShader`, `TextureShader`, `Light2DShader`, `AmbientGradientShader`).
5. **[`TE::Framebuffer`](file:///e:/TimeEngine/Engine/Include/Renderer/Framebuffer.hpp)**: Off-screen render target for editor viewports, post-processing, and screenshot captures.

---

## Key Functions & Usage Guidelines

### 1. `Renderer2D` Quad & Geometry Submission

#### `Renderer2D::BeginFrame(viewProjection)` & `EndFrame()`
- **When Used**: Call before submitting any 2D geometry during frame updates. Passes camera view-projection matrix to `RenderBatcher`.

```cpp
m_Renderer2D->BeginFrame(camera.GetViewProjectionMatrix());
m_Renderer2D->SubmitQuad(position, size, material);
m_Renderer2D->EndFrame();
```

#### `Renderer2D::SubmitLight(lightComponent, position, rotation)`
- **When Used**: Renders 2D Point, Spot, or Line light geometry with falloff exponent and inner/outer angle attenuation into the additive light buffer.

#### `Renderer2D::SubmitShadow(lightPos, lightRadius, obstacleVertices)`
- **When Used**: Projects shadow volume triangles away from 2D light sources through silhouette obstacle vertices (`SubmitTriangle`).

---

## Related Architectural Documentation

- [OpenGL Graphics Backend Architecture](file:///e:/TimeEngine/Engine/src/Renderer/OpenGL/ARCHITECTURE.md) — Cross-platform OpenGL hardware backend implementation.
- [OpenGL ES Graphics Backend Architecture](file:///e:/TimeEngine/Engine/src/Renderer/OpenGLES/ARCHITECTURE.md) — Embedded & mobile OpenGL ES hardware backend.
- [DirectX 11 Graphics Backend Architecture](file:///e:/TimeEngine/Engine/src/Renderer/DirectX11/ARCHITECTURE.md) — Direct3D 11 hardware backend implementation.
- [Vulkan Graphics Backend Architecture](file:///e:/TimeEngine/Engine/src/Renderer/Vulkan/ARCHITECTURE.md) — Explicit Vulkan 1.3 low-overhead hardware backend.
- [Material Subsystem Architecture](file:///e:/TimeEngine/Engine/src/Renderer/Material/ARCHITECTURE.md) — Node-based material pass stack and uniform caching.
- [Camera Subsystem Architecture](file:///e:/TimeEngine/Engine/src/Camera/ARCHITECTURE.md) — View and projection matrix math.
- [Window Subsystem Architecture](file:///e:/TimeEngine/Engine/src/Window/ARCHITECTURE.md) — OpenGL, DirectX & Vulkan graphics context creation.
- [Utils & TimeGUI Architecture](file:///e:/TimeEngine/Engine/src/Utils/ARCHITECTURE.md) — Color utilities (`TEColor`) and UI rendering wrappers.




