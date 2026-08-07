# Editor Subsystem Architecture

The Editor subsystem in TimeEngine powers the visual development workspace, managing editor workspace modes (`EditorMode`), modal asset editors (`AssetEditor`), toolbars (`EditorToolbar`), and GUI editor layers.

> [!NOTE]
> In short, think of the **Editor Subsystem** as the director's control room: `EditorModeRegistry` manages switching active workspace layouts (like 2D Scene Mode or Sprite Animation Mode); `AssetEditorRegistry` pops up dedicated inspector tabs when you double-click textures, materials, or sprite sheets; and `EditorToolbar` houses control buttons (Play, Pause, Stop, Mode Switchers).

---

## Component Overview & Architecture

```
                       [ EditorLayer ]
                              │
            ┌─────────────────┼─────────────────┐
            ▼                 ▼                 ▼
  [ EditorModeRegistry ] [ AssetEditorRegistry ] [ EditorToolbar ]
            │                 │
            ├──► SpriteMode   ├──► TextureAssetEditor
            ├──► DefaultModes ├──► SpriteAssetEditor
            └──► TileMapMode  └──► MaterialAssetEditor
```

### Core Subsystems

1. **Workspace Modes (`EditorMode` — `Engine/Include/Editor/EditorMode.hpp`)**:
   - Manages active editing state, viewport drawing, and toolbar icons.
   - Global registration macro: `T_REGISTER_EDITOR_MODE(MyModeClass)`.

2. **Asset Inspection & Editing (`AssetEditor` & `AssetEditorRegistry` — `Engine/Include/Editor/AssetEditorRegistry.hpp`)**:
   - Handles specialized editor tabs opened inside the editor (`EditorTab`).
   - Maps file asset types (`.png`, `.tesprite`, `.temat`) to specialized editor drawers.
   - Global registration macro: `TE_REGISTER_ASSET_EDITOR(MyAssetEditor)`.

3. **Editor Toolbar (`EditorToolbar` — `Engine/Include/Editor/EditorToolbar.hpp`)**:
   - Renders top workspace controls, play/pause state toggles, and mode buttons using `TimeGUI`.


---

## Function Roles & Usage Guidelines

### 1. Editor Mode System (`EditorModeRegistry`)

#### `EditorModeRegistry::SetActiveMode(name)`
- **When Used**: Call when switching workspace modes (e.g. from 2D Layout Mode to Sprite Mode).
- **What it does**: Safely calls `OnExit()` on the old mode, updates the active pointer, and calls `OnEnter()` on the new mode.
- **When NOT to use**: Do NOT switch modes directly during mid-render calls inside an active mode's `OnTimeGUIRender()` without deferring to next frame.

```cpp
TE::EditorModeRegistry::SetActiveMode("SpriteMode");
```

#### `EditorMode::OnUpdate(dt)` & `EditorMode::OnTimeGUIRender()`
- **`OnUpdate(dt)`**: Invoked every frame to update viewport camera, gizmo drag operations, or entity selection.
- **`OnTimeGUIRender()`**: Invoked during GUI rendering pass to draw mode-specific floating panels and gizmos using `TimeGUI`.

---

### 2. Asset Editor System (`AssetEditorRegistry`)

#### `AssetEditorRegistry::GetEditor(assetType)`
- **When Used**: Called by the Content Browser or Project Hub when a user double-clicks an asset file.

```cpp
auto editor = TE::AssetEditorRegistry::GetEditor(".temat");
if (editor) {
    editor->DrawEditor(tab);
}
```

#### `AssetEditor::DrawEditor(tab)`
- **When Used**: Called inside editor workspace tabs to draw specialized editing tools (e.g. texture UV slicer, material shader property editor).
