# TimeEngine | Resources 📦

This directory contains the static assets and identity files for the engine.

## Directory Structure

### 1. Branding (`/Branding`)
Contains the official identity assets for the TimeEngine.
- **TimeEngineIcon.png**: High-resolution branding icon used for the window icon and editor UI.
- **TimeEngineIcon.ico**: Standard Windows icon file for the executable.

### 2. Editor Assets (`/Editor`)
Contains icons used specifically for the Editor UI and Content Browser.
- **FolderIcon.png**: Icon for directory representation.
- **FileIcon.png**: Default icon for generic files.
- **LeftArrowIcon.png**: Navigation icon for back movement.
- **SaveIcon.png**: Toolbar icon for saving the current scene.
- **PlayButtonIcon.png**: Toolbar icon for entering/exiting play mode.
- **Up/Down/RightArrowIcon.png**: Additional UI navigation icons.

### 3. Game Assets (`/Assets`)
General purpose assets for game development (textures, models, etc.).

### 4. Engine Content Scripts (`/Scripts`)
Comprehensive out-of-the-box **TScript** (`.tscript`) standard library providing production-ready gameplay extensions, mechanics, and utilities:
- **Player Extensions (`/Scripts/Player`)**: Dash, stamina, double jump, and wall mechanics extensions.
- **Camera Extensions (`/Scripts/Camera`)**: Lookahead, boundary clamping, trauma shake, and zoom controllers.
- **Gameplay & Combat (`/Scripts/Gameplay`)**: Health/shield damage handler, projectile impact FX, trigger event dispatcher, proximity interactable, and wave spawner.
- **AI Behaviors (`/Scripts/AI`)**: Waypoint patrol and aggro pursuit state machines.
- **Environment Props (`/Scripts/Environment`)**: Harmonic floating bobber, oscillating rotator, moving platforms, and destructible props.
- **Utilities (`/Scripts/Utilities`)**: Timed action trigger and day/night cycle manager.
- For complete documentation and property listings, refer to **[`Resources/Scripts/README.md`](Scripts/README.md)**.

## Usage in Code
The engine expects these resources to be located in the `Resources/` directory relative to the execution path. Path lookups in the engine prioritize this folder for all UI, branding, and standard engine content elements.
