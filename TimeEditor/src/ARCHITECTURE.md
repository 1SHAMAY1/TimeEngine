# TimeEditor Application Source Architecture (`TimeEditor/src`)

The **TimeEditor Application** (`TimeEditor/src/TimeEditorApplication.cpp`) is the client-side entry point for the TimeEngine visual editor application (`TimeEditor.exe`).

> [!NOTE]
> In short, `TimeEditorApplication.cpp` implements `TE::CreateApplication()`, forces high-performance discrete GPUs (`NvOptimusEnablement`, `AmdPowerXpressRequestHighPerformance`), registers `.teproj` Win32 file associations, parses command-line project arguments, loads target graphics APIs (`RendererContext::SetAPI`), and manages the startup sequence (`LogoLayer` $\rightarrow$ `ProjectHubLayer` or `EditorLayer`).

---

## Editor Startup Pipeline & Lifecycle

```
[ OS Launch / Command-Line Arguments ]
                │
                ▼ (TE::CreateApplication)
[ High-Performance GPU Export & Win32 File Association ]
  ├─ NvOptimusEnablement / AmdPowerXpressRequestHighPerformance
  └─ PlatformUtils::RegisterFileAssociation(".teproj")
                │
                ▼ (Instantiate TimeEditor Application Class)
[ LogoLayer Startup ] (Plays Engine Animated Splash Screen)
                │
                ▼ (LogoFinishedDelegate Callback)
  ┌─────────────┴─────────────┐
  │                           │
  ▼ (Project Arg Passed)      ▼ (No Project Arg)
[ Project::Load(path) ]    [ ProjectHubLayer ]
  │                          (Project Launcher Window)
  ▼                          │
[ EditorLayer ] ◄────────────┘
(Main Visual Editor Workspace)
```

---

## Core Technical Features

### 1. Discrete GPU Forcing (`NvOptimusEnablement`)
To ensure high framerates on dual-GPU laptops (Intel HD + NVIDIA / AMD), `TimeEditorApplication.cpp` exports hardware flags directly inside the main executable:

```cpp
#ifdef _WIN32
extern "C"
{
    __declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001;
    __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}
#endif
```

---

### 2. Operating System File Association
Automatically binds `.teproj` project files to `TimeEditor.exe` in the Windows Registry using `PlatformUtils::RegisterFileAssociation()` so users can double-click project files to launch the editor directly.

```cpp
if (!TE::PlatformUtils::IsFileAssociationRegistered(".teproj", executablePath))
{
    TE::PlatformUtils::RegisterFileAssociation(
        ".teproj", "TimeEngine.Project", executablePath, "TimeEngine Project File"
    );
}
```

---

### 3. Graphics API Config Override (`ProjectSettings.ini`)
Before loading the editor workspace, `TimeEditorApplication.cpp` checks `<ProjectDir>/config/ProjectSettings.ini` to read `TargetAPI` (OpenGL, DirectX 11, Vulkan, OpenGLES) and configures `RendererContext::SetAPI()`.

---

## Related Architectural Documentation

- [TimeEditor Architecture](../ARCHITECTURE.md) — Top-level TimeEditor suite overview.
- [Editor Subsystem Architecture](../../Engine/src/Editor/ARCHITECTURE.md) — Workspace modes, asset tabs, and toolbars.
- [Layers Subsystem Architecture](../../Engine/src/Core/Layers/ARCHITECTURE.md) — `LogoLayer`, `ProjectHubLayer`, and `EditorLayer`.
- [Root Architecture Index](../../Docs/ARCHITECTURE.md) — Master TimeEngine architecture hub.

