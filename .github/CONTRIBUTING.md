# Contributing to TimeEngine

First off, thank you for considering contributing to TimeEngine! It's people like you who make it a great tool for everyone.

---

## 📜 Code of Conduct

By participating in this project, you agree to abide by our **[Code of Conduct](CODE_OF_CONDUCT.md)** (standard Contributor Covenant v2.1). Please report unacceptable behavior to **shankharajdatta2004@gmail.com** or repository maintainers.

---

## 💬 Community & Discussions

Join our community on **[GitHub Discussions](https://github.com/1SHAMAY1/TimeEngine/discussions)** to:
- Ask questions and get development help
- Propose architecture RFCs and engine enhancements
- Share game showcases and tools built with TimeEngine

---

## 🛠️ Development Setup

### 1. Clone & Initialize Submodules
```bash
git clone --recursive https://github.com/1SHAMAY1/TimeEngine.git
cd TimeEngine
bash Scripts/SetupSubmodules.sh
```

### 2. Generate Workspace & Project Files
Run the generator script for your specific platform and toolchain:
- **Windows (Visual Studio / MSVC)**: `Scripts\Windows\MSVC\GenerateProjectFiles.bat`
- **Windows (MinGW / GCC)**: `Scripts\Windows\MinGW\GenerateProjectFiles.bat`
- **Windows (LLVM Clang-cl)**: `Scripts\Windows\Clang\GenerateProjectFiles.bat`
- **macOS (Xcode)**: `bash Scripts/Mac/Xcode/GenerateProjectFiles.sh`
- **macOS (Makefiles / Apple Clang)**: `bash Scripts/Mac/Makefiles/GenerateProjectFiles.sh`
- **Linux (Makefiles / GCC & Clang)**: `bash Scripts/Linux/GenerateProjectFiles.sh`

### 3. Build the Engine & Editor
- **Windows (MSVC)**: `Scripts\Windows\MSVC\BuildDebug.bat`
- **Windows (MinGW)**: `Scripts\Windows\MinGW\BuildDebug.bat`
- **Windows (Clang)**: `Scripts\Windows\Clang\BuildDebug.bat`
- **macOS (Xcode)**: `bash Scripts/Mac/Xcode/BuildDebug.sh`
- **macOS (Makefiles)**: `bash Scripts/Mac/Makefiles/BuildDebug.sh`
- **Linux (Makefiles)**: `bash Scripts/Linux/BuildDebug.sh`

---

## 📝 Commit & Pull Request Guidelines

To maintain high commit history quality, TimeEngine runs an automated **Commit Guidelines Validator** in CI. All commits and PRs must comply with the following standards:

### 1. Commit Header Format
Every commit header must follow the format:
```text
<Tag>: <Brief Description>
```
- **Length**: Strictly **less than 50 characters** (maximum 49 characters).
- **Style**: Use imperative mood (e.g., `Editor: Fix texture inspector crash` rather than `fixed crash`).
- **No Lazy Messages**: Generic messages like `fix`, `update`, `test`, `wip`, or `changes` will fail CI validation.

### 2. Approved Canonical Tags
Your `<Tag>` must be one of the following official module tags:
> `Core`, `Renderer`, `Platform`, `Window`, `Utils`, `ECS`, `GameFrameWork`, `Input`, `Threading`, `Time`, `Asset`, `Collision`, `Events`, `Physics`, `Plugin`, `Project`, `Scene`, `Editor`, `Docs`, `CI/CD`, `Vendor`, `AI Agents`, `Automation`, `MAC`, `Linux`, `Windows`, `Release`, `Build`

### 3. Pull Request Template & Description
When submitting a Pull Request, please fill out all sections of [`.github/PULL_REQUEST_TEMPLATE.md`](PULL_REQUEST_TEMPLATE.md):
- **Description**: Detailed explanation of motivation and changes.
- **Type of Change**: Bug fix, new feature, documentation, etc.
- **Testing**: Steps taken to verify changes locally.
- **Checklist**: Confirmation of style guidelines, tests, and determinism.

### 4. Pull Request Limits & Contributor Policy
To ensure meaningful reviews, prevent review fatigue, and eliminate automated PR spam across forks:
- **1 Active PR & 24-Hour Cooldown Limit**: Contributors and fork accounts are strictly limited to **1 active Pull Request at a time**. Furthermore, a mandatory **24-hour cooldown period applies after your PR is merged or closed** before you may open a new Pull Request. This ensures careful review cycles, prevents rapid-fire submissions, and maintains sustainable maintainer bandwidth.
- **First-Time Contributors & Good First Issues**: First-time contributors can claim and submit **only one `good first issue`** to get familiar with our workflow. Once merged, future contributions should focus on standard issues, plugin features, or roadmap milestones.
- **Claiming Issues Before Starting Work**: Before writing code or submitting a PR for an existing issue, **please leave a comment on the issue first** to request assignment. This avoids duplicate efforts and confirms the intended approach with maintainers.
- **New Features & Architectural Changes**: For new features, major enhancements, or architectural proposals not covered by an open issue, **please start a thread in [GitHub Discussions](https://github.com/1SHAMAY1/TimeEngine/discussions) first** to align on design before writing code.
- **Unsolicited / Random PRs Policy**: Maintainers reserve the right to close unsolicited or uncoordinated Pull Requests without prior discussion or issue assignment.
- **Atomic & Focused Scope**: Each PR must address a single focused task or bug fix. Avoid scattershot PRs that modify dozens of unrelated files or perform bulk automated changes without context.
- **Draft PRs for WIP**: If your PR is in progress or not yet verified locally, keep it marked as a **Draft** until ready for maintainer review.

### 5. AI-Assisted Development & Quality Standards
TimeEngine embraces modern productivity tools, including AI code assistants, under our MIT license. However, automated velocity must never compromise code health or architectural integrity:
- **Full Contributor Ownership**: You are 100% accountable for every line of code you submit. Never submit AI-generated code you have not personally inspected, understood, and verified.
- **Zero Hallucinated APIs**: Submissions using invented functions, non-existent engine types, raw owning pointers (`new`/`delete`), or raw STL types where engine abstractions exist (`TEString`, `TEArray`, `TE::Ref`) will be rejected.
- **Mandatory Local Build & Run Validation**: Code must compile cleanly and pass local build validation via platform project scripts (e.g. `Scripts\Windows\MSVC\BuildDebug.bat`) before submitting a PR.
- **No Rapid Mass-Submissions**: Rapid, unverified submissions from automated generation loops will be closed immediately and marked as spam.

---

## 🎨 Code Style & Engine Conventions

We use `.clang-format` to maintain consistent C++ code style.

> [!IMPORTANT]
> **Fork Contributors**: If submitting a PR from a fork, please run `clang-format -i <modified_files>` locally on all changed files before committing and pushing, as GitHub Actions cannot auto-commit directly back to third-party forks.

- **Naming**: PascalCase for Classes, camelCase for variables (`m_` prefix for private member variables), SCREAMING_SNAKE_CASE for macros.
- **Headers**: Use `#pragma once` in all headers.
- **Indentation**: 4 spaces.
- **Local Rule Check**: You can run `premake5 check-rules` locally to verify architecture and coding safety rules.

### Memory Management & Smart Pointers
- **No Raw Owning Pointers**: Never use `new` / `delete`, `malloc` / `free`, or unmanaged raw pointers for memory ownership.
- **Unique Ownership**: Use `Scope<T>` (or `TEScope<T>`) and create instances using `CreateScope<T>(...)` (aliases for `std::unique_ptr` / `std::make_unique`). Do NOT prefix with `TE::` or `TimeEngine::`.
- **Shared Ownership**: Use `Ref<T>` (or `TERef<T>`) and create instances using `CreateRef<T>(...)` (aliases for `std::shared_ptr` / `std::make_shared`). Do NOT prefix with `TE::` or `TimeEngine::`.
- **Weak Observers**: Use `WeakRef<T>` (or `TEWeakRef<T>`) (`std::weak_ptr`) to observe objects without taking ownership and to prevent circular reference leaks.

### Engine Types & Containers Over Raw / STL Types
- **Global Namespace (No `TE::` Qualifier)**: All engine types and smart pointer aliases live in the global scope via standard engine headers. Never qualify types with `TE::` or `TimeEngine::` and never wrap engine code in `namespace TE` (`TE_NS01`-`TE_NS04` rules).
- **Strings**: Always use `TEString` (from `Utils/TEString.hpp` via `Core/PreRequisites.h`) and `TEStringView` instead of raw `std::string`, `std::wstring`, or `const char*` across engine public APIs, components, properties, and serialization.
- **Filesystem & Paths**: Always use `TEFileSystem` (`WriteAllText`, `ReadAllText`, `ForEachLine`, `Exists`) and `TEString` path methods (`GetStem()`, `GetFilename()`, `GetParentPath()`, `GetExtension()`) instead of `<filesystem>` or legacy STL path methods (`TE_MEM04`, `TE_STR01`).
- **Dynamic Arrays & Containers**: Use `TEArray<T>` (with `Add()`, `Num()`, `IsEmpty()`, `Clear()`), `TEMap<K, V>`, and `TESet<K>` instead of `std::vector`, `std::unordered_map`, or `std::unordered_set`.
- **Optional & Results**: Use `TEOption<T>` / `TENone`, `TESpan<T>`, and `TEResult<T, E>` / `TEUnexpected<E>` (from `GameFrameWork/GameplayUtils.hpp`) for clean, exception-free error handling.
- **Math & Vectors**: Always use engine math types (`TEVector2`, `TEVector`, `TEVector4`, `TEMatrix4`, `TEQuat` from `Utils/MathUtils.hpp`) rather than raw GLM or vendor math structs.

### Strict Vendor Isolation & UI Architecture
- **UI Vendor Insulation**: UI vendor libraries (`ImGui`, `ForgeUI`) must NEVER be referenced directly in public headers, `TimeGUI` public headers, or engine client code (`TE_VND08`, `TE_VND11`). All UI rendering goes through the abstract `UIAPI` interface / `TimeGUI` wrapper inside `Engine/src/UI/`.
- **Physics & Windowing Insulation**: Never leak `Velox` (`vx*`, `TE_VND01`), `GLFW` (`glfw*`, `TE_VND02`), or raw graphics API headers (`glad/glad.h`, `d3d11.h`, `vulkan/`, `Metal/`) into gameplay or editor code. Use `PhysicsWorld`, `IWindow`, and `Renderer2D` / `RenderCommand`.

### Writing Plugins & MCP Tools
- **Writing Plugins**:
  - Save descriptors as `<PluginName>.teplugin` (containing `Name: ...`, `Version: ...`, `Description: ...`, `Enabled: true/false`).
  - Place the dynamic library (`.dll` / `.so` / `.dylib`) in the plugin folder, discovered dynamically by `PluginManager`.
- **Writing MCP Tools**:
  - Register tools declaratively using the macro `TE_REGISTER_MCP_TOOL(Name, Description, SchemaJson, Handler)` or dynamically via `MCPToolRegistry::RegisterTool()`.

### Component Registration
When adding new components to the ECS (under `Engine/Include/Core/ECS/`), make sure to use reflection macros so they register with the serialization and Editor systems:
- Register the component using `T_REGISTER_COMPONENT(MyComponent, "My Component Name")`.
- Register each property using `T_REGISTER_PROPERTY(MyComponent, Type, VariableName, "Display Name")`.
This allows the Editor's **Properties** panel to draw the controls automatically without writing manual UI code.

---

## 🎯 Core Focus: 2D Excellence & Time Manipulation

Contributions that advance the **Inbuilt 2D Sprite Editor** or the **Time Manipulation** system (deterministic simulation, state snapshots, time-reversal) are highly prioritized. We aim to be the premier C++ solution for deterministic 2D development. See [Docs/ROADMAP.md](../Docs/ROADMAP.md) for details.

