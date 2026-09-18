# Project Subsystem Architecture

The Project subsystem in TimeEngine manages active project configuration (`ProjectConfig`), root/asset directory resolution, project creation/loading ([`Project`](../../../Include/Core/Project/Project.hpp)), and `.teproj` text file serialization ([`ProjectSerializer`](../../../Include/Core/Project/ProjectSerializer.hpp)).

> [!NOTE]
> In short, think of the **Project Subsystem** as the project manager's file cabinet: `Project` holds the active workspace settings (like project name, root directory, and default start scene), while `ProjectSerializer` saves and reads `.teproj` configuration files on disk.

---

## Component Overview

- **`ProjectConfig`**: Struct holding project metadata:
  - `Name`: Display name of the active project.
  - `StartScene`: Relative path to default startup scene (`.tescene`).
  - `AssetDirectory`: Path to assets folder (defaults to `Assets/`).
  - `ScriptModulePath`: Path to compiled game C++ dynamic library.
  - `ThumbnailPath`: Path to project preview thumbnail image.
- **`Project`**: Static manager class maintaining `s_ActiveProject`.
- **`ProjectSerializer`**: Text/YAML serializer reading and writing key-value configuration lines from `.teproj` files.

---

## Core Functions & Usage Guidelines

### 1. `Project::New()`
- **When Used**: Invoked by the Project Hub layer when a user creates a new empty project.
- **What it does**: Instantiates an empty `Project` instance with default configuration values (`Assets/` directory).

```cpp
auto newProj = TE::Project::New();
newProj->GetConfig().Name = "MyGame";
```

### 2. `Project::Load(filepath)`
- **When Used**: Called when opening an existing project from disk via Project Hub or command line.
- **What it does**: Reads `.teproj` via `ProjectSerializer::Deserialize()`, sets `s_ActiveProject`, and computes absolute asset directory paths.

```cpp
TE::Ref<TE::Project> project = TE::Project::Load("C:/MyGames/SpaceShooter/SpaceShooter.teproj");
if (project) {
    TE_CORE_INFO("Loaded project: {0}", project->GetConfig().Name);
}
```

### 3. `Project::SaveActive(filepath)`
- **When Used**: Invoked by the Editor when saving project settings or closing the workspace.

```cpp
TE::Project::SaveActive("C:/MyGames/SpaceShooter/SpaceShooter.teproj");
```

### 4. `Project::GetAssetDirectory()`
- **When Used**: Call whenever resolving asset paths inside `AssetManager` or Content Browser.
- **Behavior**: Automatically resolves relative paths against the project root directory `m_ProjectDirectory`.

```cpp
// Returns absolute path e.g. "C:/MyGames/SpaceShooter/Assets"
std::filesystem::path assetDir = TE::Project::GetAssetDirectory();
```
