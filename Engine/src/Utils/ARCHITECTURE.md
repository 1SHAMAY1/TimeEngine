# Utils Subsystem Architecture

The `Utils` module in TimeEngine provides foundational, vendor-decoupled utility classes and platform abstractions used across the core engine, rendering system, and editor layer.

> [!NOTE]
> In short, think of the **Utils Subsystem** as the engine's primary toolbox: `MathUtils` provides essential 2D vector & transformation math primitives so sprites and entities move smoothly; `TimeGUI` draws buttons, sliders, and editor menus without tying the engine to any specific third-party library brand; and `PlatformUtils` acts as a diplomat talking to Windows/Unix operating systems natively.



---

## Platform Utilities Link Reference

- [Platform Utilities Architecture](Platform/ARCHITECTURE.md) — Architectural documentation for OS-specific platform utilities.

---

## 1. Math Utilities (`MathUtils`)

`TE::MathUtils` (`Engine/Include/Utils/MathUtils.hpp`) defines engine math primitives, avoiding hard vendor dependencies in engine header files.


### Key Types & Conversion Overloads
- **`TEVector2` / `TEVector` / `TEVector4`**: 2D, 3D, and 4D float vectors (with primary 2D focus for engine transforms, UI alignment, and sprite positioning) equipped with magnitude, normalization, dot/cross products, linear interpolation (`Lerp`), and seamless conversion to/from UI types (`ImVec2`, `ImVec4`).
- **`TEMatrix4`**: 4x4 matrix struct with matrix-matrix multiplication, vector transform, and 2D orthographic projection generators (`TEMatrix4::Ortho`).
- **`TERotator` & `TEQuat`**: Rotator storing Euler angles (`Pitch`, `Yaw`, and `Roll`) with conversion to quaternions (`ToQuat()`) and rotation matrices.
- **`TETransform`**: Standard transform structure bundling `Position`, `Rotation`, and `Scale`.


```cpp
// Conversion & Operator Excerpt from MathUtils.hpp
struct TE_API TEVector2
{
    float x = 0.0f, y = 0.0f;
    float Length() const { return std::sqrt(x * x + y * y); }
    TEVector2 Normalized() const { ... }
    
    // Vendor-agnostic implicit conversion to ImGui ImVec2
    ImVec2 ToImVec2() const;
    operator ImVec2() const;
};

inline TEVector Cross(const TEVector &a, const TEVector &b)
{
    return {a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x};
}
```

---

## 2. TimeGUI Abstraction Layer (`TimeGUI`)

[`TE::TimeGUI`](file:///e:/TimeEngine/Engine/Include/Utils/TimeGUI.hpp) wraps Dear ImGui behind clean TimeEngine-native types. This shields engine code from third-party ImGui header leaks and allows switching or extending the UI frontend transparently.

### Architecture Highlights
- **Opaque Handles & Cast Proxies**: Types like `TimeGUIFont`, `TimeGUIDrawList`, and `TimeGUITextureID` (`void*`) wrap third-party pointers safely:
```cpp
struct TE_API TimeGUIDrawList
{
    void *nativeDrawList = nullptr;
    void AddLine(const TEVector2 &p1, const TEVector2 &p2, unsigned int color, float thickness = 1.0f);
    void AddRectFilled(const TEVector2 &p1, const TEVector2 &p2, unsigned int color, float rounding = 0.0f);
    ::ImDrawList *operator->() const { return static_cast<::ImDrawList *>(nativeDrawList); }
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

