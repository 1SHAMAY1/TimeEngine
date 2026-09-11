# Utils Subsystem Architecture

The `Utils` module in TimeEngine provides foundational, vendor-decoupled utility classes and platform abstractions used across the core engine, rendering system, and editor layer.

> [!NOTE]
> In short, think of the **Utils Subsystem** as the engine's primary toolbox: `MathUtils` provides essential 2D vector & transformation math primitives so sprites and entities move smoothly; `TimeGUI` draws buttons, sliders, and editor menus without tying the engine to any specific third-party library brand; and `PlatformUtils` acts as a diplomat talking to Windows/Unix operating systems natively.



---

## Platform Utilities Link Reference

- [Platform Utilities Architecture](Platform/ARCHITECTURE.md) — Architectural documentation for OS-specific platform utilities.

---

## 1. Math Subsystem (`MathAPI` & `MathUtils`)

`TE::MathEngine` (`Engine/Include/Utils/Math/MathEngine.hpp`) and `TE::MathAPI` (`Engine/Include/Utils/Math/MathAPI.hpp`) provide an abstract, backend-switchable linear algebra engine. High-level primitives in `MathUtils.hpp` delegate all matrix multiplications, quaternions, and projection computations to the active `MathAPI` backend (`GLMMathAPI` or `CustomMathAPI`).

### Key Types & Clean Architecture
- **`TEVector2` / `TEVector` / `TEVector4`**: Pure float vector value types equipped with magnitude, normalization, dot/cross products, and linear interpolation (`Lerp`).
- **`TEMatrix4`**: 4x4 matrix struct with multiplication, transformations, and orthographic/perspective projections dispatched via `MathAPI`.
- **`TERotator` & `TEQuat`**: Rotator storing Euler angles (`Pitch`, `Yaw`, and `Roll`) with conversion to quaternions (`ToQuat()`) and rotation matrices via `MathAPI`.
- **`TETransform`**: Standard transform structure bundling `Position`, `Rotation`, and `Scale`.
- **`ProjectMathSettings`**: Project settings page under "Core" $\rightarrow$ "Math Engine & Backend" enabling runtime backend switching.

```cpp
// Pure engine math types in MathUtils.hpp - Zero Vendor Leakage
struct TE_API TEVector2
{
    float x = 0.0f, y = 0.0f;
    float Length() const { return std::sqrt(x * x + y * y); }
    TEVector2 Normalized() const { ... }
};

inline TEVector Cross(const TEVector &a, const TEVector &b)
{
    return {a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x};
}
```

---

## 2. TimeGUI Abstraction Layer (`TimeGUI`)

[`TE::TimeGUI`](../../Include/Utils/TimeGUI.hpp) acts as the high-level static facade wrapping immediate-mode GUI operations behind clean TimeEngine-native types (`TEVector2`, `TEVector4`, `TEColor`, `TEString`).

### Architecture & UI Subsystem Delegation
- **Delegated Subsystem Pipeline**: Lifecycle calls (`Init`, `Shutdown`, `InitOpenGLBackend`, `ShutdownOpenGLBackend`, `BindWidgetThreadContext`, `PrepareGLFWFrame`, `BeginFrame`, `EndFrame`, `RenderDrawData`) delegate directly to the [`UIEngine`](../UI/ARCHITECTURE.md) subsystem and its active [`UIAPI`](../../Include/UI/UIAPI.hpp) backend (`ForgeUIAPI` or `ImGuiUIAPI`).
- **Vendor Insulation**: Shields engine code, editor inspectors, and game plugins from third-party GUI headers.
- **Opaque Handles & Cast Proxies**: Types like `TimeGUIFont`, `TimeGUIDrawList`, and `TimeGUITextureID` wrap native draw data safely:
```cpp
struct TE_API TimeGUIDrawList
{
    void *nativeDrawList = nullptr;
    void AddLine(const TEVector2 &p1, const TEVector2 &p2, unsigned int color, float thickness = 1.0f);
    void AddRectFilled(const TEVector2 &p1, const TEVector2 &p2, unsigned int color, float rounding = 0.0f);
    template <typename T> T *As() const { return static_cast<T *>(nativeDrawList); }
};
```
- **Engine-Native Enums**: Wraps `ImGuiWindowFlags`, `ImGuiCol`, `ImGuiStyleVar`, and `ImGuiKey` into `TimeGUIWindowFlags`, `TimeGUICol`, `TimeGUIStyleVar`, and `TimeGUIKey`.
- **Decoupled API**: Exposes windowing (`Begin`, `End`), widgets (`Button`, `SliderFloat`, `InputText`, `ColorPicker4`), layout helpers (`SameLine`, `Columns`), and draw list primitives (`GetWindowDrawList`).

---

## When to Use Which Utility (Beginner Guide)

| Task | Use This Subsystem | Function / Type |
|---|---|---|
| Moving or positioning a 2D sprite | **`MathUtils`** | Use `TEVector2` for 2D position/velocity and `TETransform` for full transform matrix calculation (`GetMatrix()`). |
| Interpolating between two points | **`MathUtils`** | Use `TE::Lerp(a, b, t)` for smooth movement or fade transitions. |
| Drawing an editor button or text field | **`TimeGUI`** | Use `TE::TimeGUI::Button("Label")` or `TE::TimeGUI::InputText("Label", str)`. |
| Drawing custom shapes on a UI panel | **`TimeGUI`** | Fetch `TE::TimeGUI::GetWindowDrawList()` and call `AddLine`, `AddRectFilled`, or `AddCircle`. |
| Opening a native file/folder dialog | **`PlatformUtils`** | Call `TE::PlatformUtils::OpenFile("*.png")` or `TE::PlatformUtils::OpenFolder()`. |

