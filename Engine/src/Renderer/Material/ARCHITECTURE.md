# Material Subsystem Architecture

The Material subsystem in TimeEngine provides modular surface shaders, uniform cache binding ([`Material`](file:///e:/TimeEngine/Engine/Include/Renderer/Material.hpp)), material instantiation overrides ([`MaterialInstance`](file:///e:/TimeEngine/Engine/Include/Renderer/MaterialInstance.hpp)), a global material repository ([`MaterialLibrary`](file:///e:/TimeEngine/Engine/Include/Renderer/MaterialLibrary.hpp)), node-based pass stacks (`MaterialPassNode` & `MaterialPassNodeType`), and `.tematerial` file serialization ([`MaterialSerializer`](file:///e:/TimeEngine/Engine/src/Renderer/MaterialSerializer.cpp)).

> [!NOTE]
> In short, think of the **Material Subsystem** as the surface texture and shader paint customizer: `Material` holds the shader uniform values (float, int, vec2, vec3, vec4, mat4) and multi-node pass stack (PBR slabs, normal maps, emissive glows, dissolve masks, UV scrollers); `MaterialInstance` allows creating dynamic material copies with custom color overrides without recompiling shaders; while `MaterialSerializer` saves and loads `.tematerial` files on disk.

---

## Subsystem Architecture & Node Pipeline

```
[ Material Asset (.tematerial) ]
               │
               ▼ (Bound to Shader via ApplyUniforms)
[ Material Class ]
               │
               ├──► Uniform Caches: Float, Int, Vec2, Vec3, Vec4, Mat4
               ├──► Shader Handle: std::shared_ptr<Shader>
               └──► Material Pass Stack (vector<MaterialPassNode>)
                         │
                         ├──► Base Surface Slab
                         ├──► PBR Metallic / Roughness Slab
                         ├──► Coat Normal / Bump Slab
                         ├──► Emissive Glow Slab
                         ├──► UV Scroller Modifier
                         └──► Dissolve Mask / Fresnel Rim Modifier
               │
               ▼ (Per-Object Material Instance Overrides)
[ MaterialInstance ] ──► (Overrides Color/Uniforms without modifying Base Material)
```

---

## Core Classes & Subsystem Roles

1. **[`TE::Material`](file:///e:/TimeEngine/Engine/Include/Renderer/Material.hpp)**:
   - Derives from `Asset` (`.tematerial`).
   - Stores shader reference `m_Shader` and strongly-typed uniform maps (`m_FloatUniforms`, `m_IntUniforms`, `m_Vec2Uniforms`, `m_Vec3Uniforms`, `m_Vec4Uniforms`, `m_Mat4Uniforms`).
   - Maintains node-based pass stack (`m_PassStack`) for multi-pass surface shader effects.

2. **[`TE::MaterialInstance`](file:///e:/TimeEngine/Engine/Include/Renderer/MaterialInstance.hpp)**:
   - Light weight wrapper referencing a shared base `Material`.
   - Allows setting instance-specific properties (e.g. `m_OverrideColor`) for individual mesh renderers without allocating a new `Material` instance.

3. **[`TE::MaterialLibrary`](file:///e:/TimeEngine/Engine/Include/Renderer/MaterialLibrary.hpp)**:
   - Global material registry (`Register`, `Get`, `Exists`) caching named default materials (e.g. `DefaultMaterial`, `Light2DMaterial`).

4. **[`TE::MaterialSerializer`](file:///e:/TimeEngine/Engine/src/Renderer/MaterialSerializer.cpp)**:
   - Text serializer reading and writing `.tematerial` files on disk.
   - Formats material name, RGBA color, and `MaterialPassNode` attributes.

---

## Material Pass Node System (`MaterialPassNodeType`)

The `Material` pass stack allows combining modular surface modifiers:

- **`BaseSurfaceSlab`**: Primary diffuse texture, tint color, and alpha blend mode.
- **`PBRMetallicRoughnessSlab`**: Physical rendering parameters (Metallic, Roughness values).
- **`CoatNormalSlab`**: Normal map texture path, bump depth, and tiling parameters.
- **`EmissiveGlowSlab`**: Self-illumination color and glow intensity multiplier.
- **`UVScrollerModifier`**: Dynamic UV offset animation speeds ($X, Y$).
- **`FresnelRimLightModifier`**: Edge rim lighting intensity and exponent.
- **`DissolveMaskModifier`**: Noise texture dissolve threshold and edge burn color.

---

## Core Functions & Usage Guidelines

### 1. `Material::SetUniform(...)` & `ApplyUniforms()`
- **When Used**: Set shader uniform values on a material instance before submitting to `Renderer2D`.
- **Behavior**: Caches uniform values in internal maps and uploads them to the GPU when `ApplyUniforms()` or `m_Shader->Bind()` is executed.

```cpp
auto material = std::make_shared<TE::Material>(shader);
material->SetColor(TEColor::Red());
material->SetUniform("u_Intensity", 2.5f);
material->SetUniform("u_Tiling", glm::vec2(2.0f, 2.0f));
```

---

### 2. Creating Material Pass Nodes (`AddPassNode`)
- **When Used**: Invoked by `MaterialAssetEditor` in the editor inspector to stack multi-pass shader nodes.

```cpp
TE::MaterialPassNode glowNode;
glowNode.Name = "Emissive Glow";
glowNode.Type = TE::MaterialPassNodeType::EmissiveGlowSlab;
glowNode.Color = TEVector4(1.0f, 0.8f, 0.0f, 1.0f);
glowNode.FloatVal1 = 5.0f; // Glow Intensity

material->AddPassNode(glowNode);
```

---

### 3. Lightweight Instancing (`MaterialInstance`)
- **When Used**: Use `MaterialInstance` when multiple scene entities share the same shader and base texture, but require distinct color tints.

```cpp
auto instance = std::make_shared<TE::MaterialInstance>(baseMaterial);
instance->SetColor(TEColor::Green());
instance->ApplyUniforms();
```

---

## Related Architectural Documentation

- [Renderer Subsystem Architecture](../ARCHITECTURE.md) — View-projection matrix binding in batch rendering.
- [Editor & Asset Editors Architecture](../../Editor/ARCHITECTURE.md) — Documentation for `MaterialAssetEditor` tab.
- [Asset Subsystem Architecture](../../Core/Asset/ARCHITECTURE.md) — Asset handle and `.tematerial` path registration.

