# Windows Platform Utilities Architecture

The `WindowsPlatformUtils.cpp` module implements native Windows OS features for file dialogs, folder pickers, file associations, and executable path resolution.

> [!NOTE]
> In short, this module acts as the Windows-specific diplomat for TimeEngine: it talks directly to the Windows OS to open native file/folder pickers, register project file extensions in the Windows Registry, and locate the running executable on disk.

---

## Key APIs & Usage Guidelines

### 1. `PlatformUtils::OpenFolder(initialPath)`
- **When Used**: Call when the user clicks "Open Project" or selects asset output directories in the Editor UI.
- **When NOT to use**: Do NOT use for selecting single files (use `OpenFile`).

```cpp
std::string path = PlatformUtils::OpenFolder();
if (!path.empty()) { /* Open project at folder path */ }
```

### 2. `PlatformUtils::OpenFile(filter)` & `PlatformUtils::SaveFile(filter)`
- **When Used**: Call when importing textures/models (`OpenFile("PNG Files (*.png)\0*.png\0")`) or exporting project scenes (`SaveFile`).
- **When NOT to use**: Do NOT hardcode absolute paths in filters; always pass proper Win32 double-null terminated strings.

```cpp
std::string filePath = PlatformUtils::OpenFile("TimeEngine Project (*.teproject)\0*.teproject\0");
```

### 3. `PlatformUtils::RegisterFileAssociation(...)`
- **When Used**: Call during editor setup or settings initialization to associate `.teproject` or `.tesprite` file extensions with TimeEngine binary in Windows Registry (`HKCU`).
- **When NOT to use**: Avoid calling every frame; call once on app startup or user configuration toggle.

### 4. `PlatformUtils::GetExecutablePath()`
- **When Used**: Call when locating relative engine resources (`Resources/`) or launching sub-processes.

