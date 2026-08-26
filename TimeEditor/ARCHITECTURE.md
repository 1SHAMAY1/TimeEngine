# TimeEditor IGDE Suite Architecture

The `TimeEditor` directory contains the Integrated Game Development Environment (IGDE) application (`TimeEditor.exe`), which embeds the TimeEngine core runtime, dockable editor workspace panels, project creation launcher, and asset inspection tools.

> [!NOTE]
> In short, **TimeEditor** is the main desktop software application developers use to create games: it handles project launching (`ProjectHubLayer`), scene editing (`EditorLayer`), workspace mode switching (`EditorModeRegistry`), material/sprite file editing tabs (`AssetEditorRegistry`), real-time performance profiling, and OS file associations (`.teproj`).

---

## 📂 Subsystem Breakdown & Architecture Links

* 🚀 **[Editor Application Entry & Lifecycle](src/ARCHITECTURE.md)** (`TimeEditor/src/`) — Client entry point (`TimeEditorApplication.cpp`), high-performance discrete GPU exports, `.teproj` Win32 file extension registration, and `LogoLayer` $\rightarrow$ `ProjectHubLayer` $\rightarrow$ `EditorLayer` startup sequence.
* 🛠️ **[Engine Editor Subsystem](../Engine/src/Editor/ARCHITECTURE.md)** — Workspace modes (`EditorModeRegistry`), modular UI widgets (`UIWidget`), universal save system (`EditorSaveManager`), layout manager (`EditorLayoutManager`), and toolbar controls (`EditorToolbar`).
* 🥞 **[Layers Architecture](../Engine/src/Core/Layers/ARCHITECTURE.md)** — `ProjectHubLayer` launcher, `EditorLayer` workspace, `ProfilingLayer`, `EngineSettingsLayer`, and `TimeGUILayer`.
* 🏠 **[Root Architecture Index](../ARCHITECTURE.md)** — Master TimeEngine architecture index.


---

## 🏛️ TimeEditor Directory Structure

```
TimeEditor/
├── src/
│   ├── ARCHITECTURE.md                 # Editor application source architecture documentation
│   └── TimeEditorApplication.cpp      # Client entry point (CreateApplication, GPU exports, project loader)
├── ARCHITECTURE.md                     # Top-level TimeEditor suite architecture reference
├── Makefile                            # GNU Makefile for Linux/macOS editor builds
└── TimeEditor.vcxproj                  # MSVC project file for Windows Visual Studio builds
```

---

## 🚀 Key Application Execution Phases

1. **Discrete GPU Activation**: Forces discrete NVIDIA (`NvOptimusEnablement`) / AMD (`AmdPowerXpressRequestHighPerformance`) graphics cards on dual-GPU laptops.
2. **Win32 File Association**: Registers `.teproj` file handlers in the Windows Registry via `PlatformUtils`.
3. **Animated Splash Screen**: Displays `LogoLayer` and waits for `LogoFinishedDelegate`.
4. **Project Launcher vs Editor Workspace**: Automatically opens `ProjectHubLayer` if no command-line argument is passed, or loads `EditorLayer` directly if a valid `.teproj` path is supplied.
