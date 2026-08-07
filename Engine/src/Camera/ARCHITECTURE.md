# Camera Subsystem Architecture

The Camera subsystem in TimeEngine provides 2D/3D projection matrices ([`Camera`](file:///e:/TimeEngine/Engine/Include/Camera/Camera.hpp)), orthographic view management ([`OrthographicCamera`](file:///e:/TimeEngine/Engine/Include/Camera/OrthographicCamera.hpp)), 3D perspective projection ([`PerspectiveCamera`](file:///e:/TimeEngine/Engine/Include/Camera/PerspectiveCamera.hpp)), rotation controllers ([`CameraController`](file:///e:/TimeEngine/Engine/Include/Camera/CameraController.hpp)), and gameplay camera tracking ([`PlayerCameraComponent`](file:///e:/TimeEngine/Engine/Include/Core/Scene/PlayerCameraComponent.hpp)).

> [!NOTE]
> In short, think of the **Camera Subsystem** as the director's lens: `Camera` calculates projection and view matrices ($\mathbf{M}_{view}$ and $\mathbf{M}_{proj}$) to transform 2D/3D world coordinates into screen coordinates, `OrthographicCamera` handles 2D pixel-perfect tile and sprite rendering, while `PlayerCameraComponent` smoothly tracks player entities in scenes.

---

## Subsystem Pipeline & Data Flow

```
[ Camera Position & Rotation ]
         │
         ▼ (Recalculate View Matrix)
[ View Matrix (M_view) ] ──┐
                           ├─► Multiply: M_viewProj = M_proj * M_view
[ Projection Matrix (M_proj) ] ┘
         │
         ▼ (Uploaded to Shaders)
[ Renderer2D / Renderer3D ] ──► [ Screen Space Viewport ]
```

---

## Core Classes & Subsystem Roles

1. **[`TE::Camera`](file:///e:/TimeEngine/Engine/Include/Camera/Camera.hpp)**: Abstract base class storing `m_ProjectionMatrix`, `m_ViewMatrix`, and combined `m_ViewProjectionMatrix`.
2. **[`TE::OrthographicCamera`](file:///e:/TimeEngine/Engine/Include/Camera/OrthographicCamera.hpp)**: 2D orthographic camera with position ($X, Y, Z$), rotation angle, and zoom scale factors.
3. **[`TE::PerspectiveCamera`](file:///e:/TimeEngine/Engine/Include/Camera/PerspectiveCamera.hpp)**: 3D perspective camera supporting Field of View ($\text{FOV}$), aspect ratio, and Euler angles (pitch, yaw, roll).
4. **[`TE::CameraController`](file:///e:/TimeEngine/Engine/Include/Camera/CameraController.hpp)**: Rotation and FOV input controller.
5. **[`TE::PlayerCameraComponent`](file:///e:/TimeEngine/Engine/Include/Core/Scene/PlayerCameraComponent.hpp)**: ECS component providing smooth player tracking (`SmoothSpeed`), camera shake effects, and follow offsets.

---

## Core Functions & Usage Guidelines

### 1. `OrthographicCamera` (2D Rendering)

#### `OrthographicCamera::SetProjection(left, right, bottom, top)`
- **When Used**: Invoked during viewport resize events to update the 2D orthographic bounds:
  $$\mathbf{M}_{proj} = \text{glm::ortho}(left \cdot zoom, right \cdot zoom, bottom \cdot zoom, top \cdot zoom, -1.0, 1.0)$$

#### `OrthographicCamera::RecalculateViewMatrix()`
- **When Used**: Automatically called when position, rotation, or zoom changes.
- **Formula**:
  $$\mathbf{M}_{transform} = \mathbf{T}(pos) \cdot \mathbf{R}_z(rot)$$
  $$\mathbf{M}_{view} = \mathbf{M}_{transform}^{-1}$$
  $$\mathbf{M}_{viewProj} = \mathbf{M}_{proj} \cdot \mathbf{M}_{view}$$

```cpp
// Create 2D camera with 16:9 aspect ratio bounds
TE::OrthographicCamera camera(-1.6f, 1.6f, -0.9f, 0.9f);
camera.SetPosition({0.0f, 0.0f, 0.0f});
```

---

### 2. `PerspectiveCamera` (3D Rendering)

#### `PerspectiveCamera::SetProjection(fovDegrees, aspectRatio, nearClip, farClip)`
- **When Used**: Set up 3D perspective view projections.

```cpp
// 45 deg FOV, 16:9 aspect ratio, 0.1f near clip, 1000.0f far clip
TE::PerspectiveCamera camera3D(45.0f, 16.0f / 9.0f, 0.1f, 1000.0f);
```

---

### 3. `PlayerCameraComponent` (ECS Player Tracking)

#### `PlayerCameraComponent::GetCalculatedCameraPosition()`
- **When Used**: Called every frame inside game scenes to interpolate camera position towards player entity (`bFollowOwner`) using linear interpolation (`SmoothSpeed`).

```cpp
// Attach camera follower to player entity
auto* playerCam = playerEntity.AddComponent<TE::PlayerCameraComponent>();
playerCam->bFollowOwner = true;
playerCam->SmoothSpeed = 5.0f;
```

---

## Related Architectural Documentation

- [Renderer2D Subsystem Architecture](file:///e:/TimeEngine/Engine/src/Renderer/ARCHITECTURE.md) — View-projection matrix binding in batch rendering.
- [Scene & ECS Architecture](file:///e:/TimeEngine/Engine/src/Core/Scene/ARCHITECTURE.md) — Attaching `PlayerCameraComponent` to scene entities.
- [Layers Subsystem Architecture](file:///e:/TimeEngine/Engine/src/Core/Layers/ARCHITECTURE.md) — `CameraLayer` viewport navigation.
