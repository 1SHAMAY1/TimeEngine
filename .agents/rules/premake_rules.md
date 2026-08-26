# TimeEngine Premake Code Quality, Type & Memory Safety Rules

This document details the code rules and Premake actions available via `premake5` located in `Scripts/Premake/Rules/` and loaded by `Scripts/Premake/Rules.lua`.

---

## Available Premake Actions

```bash
# Master Check: Runs all safety, isolation, slop, and copyright checks
premake5 check-rules

# TimeEngine Types & Memory Safety Rule
premake5 check-memory-safety

# Strict Vendor Isolation Rule
premake5 check-vendor-leakage

# AI Slop & Boilerplate Rule
premake5 check-ai-slop

# Copyright & Competitor Detector Rule
premake5 check-copyright
```

---

## Rule Specifications

### 1. 🧠 TimeEngine Types & Memory Safety Rule ([`EngineTypesAndMemorySafetyRule.lua`](file:///e:/TimeEngine/Scripts/Premake/Rules/EngineTypesAndMemorySafetyRule.lua))
- **`TE_MEM01`**: Prohibits direct `new`. Enforces `CreateRef<T>()` or `CreateScope<T>()`.
- **`TE_MEM02`**: Prohibits direct `delete`. Enforces TimeEngine smart pointers (`Ref<T>` / `TERef<T>`, `Scope<T>` / `TEScope<T>`, `WeakRef<T>` / `TEWeakRef<T>`).
- **`TE_MEM03`**: Prohibits `std::string`, `std::wstring`, `std::string_view`. Enforces TimeEngine `TEString` across all files (exempting `TEString.hpp` / `TEString.cpp`).

### 2. 🛡️ Strict Vendor Isolation Rule ([`VendorLeakageRule.lua`](file:///e:/TimeEngine/Scripts/Premake/Rules/VendorLeakageRule.lua))
- **ImGui** (`TE_VND08`): **Strictly allowed ONLY in `Utils/TimeGUI`** (`Engine/src/Utils/TimeGUI.*` & `Engine/Include/Utils/TimeGUI.hpp`) and `Engine/src/Utils/MathUtils.cpp` for vector type bridging. All UI and Editor layers must use the `TimeGUI` wrapper.
- **stb_image** (`TE_VND09`): **Strictly allowed ONLY in Asset Manager** (`Engine/src/Core/Asset/*` & `Engine/Include/Core/Asset/*`).
- **OpenGL / GLAD** (`TE_VND03`): **Strictly allowed ONLY in `Renderer/OpenGL/`**.
- **OpenGL ES** (`TE_VND04`): **Strictly allowed ONLY in `Renderer/OpenGLES/`**.
- **Vulkan / Volk** (`TE_VND05`): **Strictly allowed ONLY in `Renderer/Vulkan/`**.
- **DirectX 11** (`TE_VND06`): **Strictly allowed ONLY in `Renderer/DirectX11/`**.
- **Metal** (`TE_VND07`): **Strictly allowed ONLY in `Renderer/Metal/`**.
- **Velox Physics** (`TE_VND01`): **Strictly allowed ONLY in `Core/Physics/`**.
- **GLFW** (`TE_VND02`): **Strictly allowed ONLY in `Window/` and `Platform/`**.
- **Customizable_Logger** (`TE_VND10`): **Strictly allowed ONLY in `Utils/Logger`**.

### 3. 🤖 AI Slop & Boilerplate Rule ([`AISlopRule.lua`](file:///e:/TimeEngine/Scripts/Premake/Rules/AISlopRule.lua))
- Scans for LLM prompt echoes, conversational filler (`as per user request`, `as an ai language model`, `certainly! here is`), and lazy omission stubs (`... existing code ...`, `todo: insert your logic here`).

### 4. ⚖️ Copyright & Competitor Mentions Rule ([`CopyrightDetectorRule.lua`](file:///e:/TimeEngine/Scripts/Premake/Rules/CopyrightDetectorRule.lua))
- Flags mentions of competitor engines/products (`Unreal Engine`, `UE5`, `Fortnite/UEFN`, `Unity`, `Godot`, `Frostbite`, `CryEngine`, `Source 2`) and proprietary subsystem classes (`Nanite`, `Lumen`, `Blueprints`, `MonoBehaviour`, `ScriptableObject`, `GDScript`), foreign copyright notices, and license anomalies.
