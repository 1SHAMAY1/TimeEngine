# MCPPlugin Architecture

The `MCPPlugin` module provides a Model Context Protocol (MCP) HTTP/SSE server (conforming to MCP specification `2024-11-05`) embedded directly inside TimeEngine. This enables external AI agents to inspect, control, and manipulate engine state dynamically via local HTTP/SSE requests.

> [!NOTE]
> In short, imagine TimeEngine as a game studio building a game in real-time. The **MCP Plugin** is like a smart remote control receiver installed in the studio that listens on a 24/7 hotline (`http://127.0.0.1:3000`). When an external AI assistant wants to check what objects are in the game or create a new game character, it sends a text message over the hotline. The MCP Plugin receives the text, translates it into engine commands, creates the character, snaps a picture of the screen if asked, and texts back the result.


---

## Component Overview

- **`TE::IPlugin` Implementation**: `MCPPlugin` (`src/MCPPlugin.hpp`, `src/MCPPlugin.cpp`).
- **Protocol**: HTTP/1.1 + Server-Sent Events (SSE) with JSON-RPC 2.0.
- **Port**: Listens on `http://127.0.0.1:3000`.

## Architecture & Data Flow

```
[ External AI Agent ]
         │
         ├─── 1. GET /sse ──────────────► [ ServerThreadMain (Listen Loop) ]
         │    (Opens SSE Stream)                         │
         │    ◄── event: endpoint ──────────────────────┤ (Registers SSE Client)
         │                                               │
         ├─── 2. POST /message ──────────────────────────┤ (Dispatches JSON-RPC)
         │    (JSON-RPC 2.0 Request)                     │
         │                                               ▼
         │                                     [ DispatchToolCall ]
         │                                               │
         │                                               ├─► Engine Info / Hierarchy
         │                                               ├─► Scene & Entity Mutation
         │                                               ├─► Asset / File System Operations
         │                                               └─► Editor Mode & Viewport Capture
         │                                               │
         │    ◄── 200 OK + JSON-RPC Result ──────────────┤
         └─── ◄── Broadcast event: message (SSE) ────────┘
```

## Endpoints

1. **`GET /sse`**:
   - Opens persistent Server-Sent Event stream for client notifications.
   - Responded with an initial `endpoint` event directing JSON-RPC POST requests to `/message`.
   - Sends periodic keep-alive pings (`: ping\n\n`) every 15 seconds.
2. **`POST /message`**:
   - Primary endpoint for client JSON-RPC 2.0 requests.
   - Handles `initialize`, `notifications/initialized`, `tools/list`, and `tools/call`.
3. **`GET /health`**:
   - Lightweight health check endpoint returning HTTP 200 OK (`OK`).
4. **`OPTIONS /message`**:
   - CORS preflight response for browser/web-based AI tooling.

## Tool Invocation & Helper Scripts

Tools exposed by `MCPPlugin` can be called either directly via HTTP POST requests to `http://127.0.0.1:3000/message` carrying JSON-RPC 2.0 payloads or through the provided CLI wrapper script:

- **Shell Wrapper**: [`Scripts/MCP_Tools.sh`](../../../Scripts/MCP_Tools.sh)
  - Usage: `./Scripts/MCP_Tools.sh <command_name> [args...]`
  - Maps simplified CLI commands to JSON-RPC 2.0 tool requests via `curl`.
  - Commands include `info`, `get_scene`, `create_entity <name>`, `destroy_entity <id>`, `create_sprite <name> <path>`, `mkdir <path>`, `rm <path>`, `get_modes`, `set_mode <name>`, `screenshot`, `delete_screenshot`, `send_key <code>`, `send_click <code>`, `select <id>`, `set_properties <id> <json>`, and `add_component <id> <type>`.

- **JSON Helper**: Uses [`Scripts/parse_json.py`](../../../Scripts/parse_json.py) for constructing structured JSON property payloads for complex entity modifications.


## Provided MCP Tools & Function Handlers

Each tool requested by an external AI agent maps to an internal C++ member function in `MCPPlugin`:

### 1. Engine & Scene Inspection
- **`Tool_GetEngineInfo()`**: Queries `Project::GetActive()` and engine settings to return active project name and directory.
```cpp
std::string MCPPlugin::Tool_GetEngineInfo() {
    std::string projectName = Project::GetActive() ? Project::GetActiveConfig().Name : "None";
    return "\"TimeEngine v1.0 | Project: " + projectName + "\"";
}
```
- **`Tool_GetSceneHierarchy()`**: Iterates active scene entities in `EditorLayer` and returns entity IDs with tags.
```cpp
std::string MCPPlugin::Tool_GetSceneHierarchy() {
    auto scene = editorLayer->GetActiveScene();
    for (EntityID id : scene->GetEntityManager().GetAliveEntities()) {
        ss << "\\n- [" << id << "] " << entity.GetComponent<TagComponent>()->Tag;
    }
}
```

### 2. Entity Management
- **`Tool_CreateEntity(paramsJson)`**: Instantiates a new entity in active scene with `TagComponent`.
```cpp
std::string MCPPlugin::Tool_CreateEntity(const std::string &paramsJson) {
    std::string name = ExtractString(paramsJson, "name");
    Entity e = scene->CreateEntity(name.empty() ? "NewEntity" : name);
    return "\"Created entity '" + name + "' with ID: " + std::to_string(e.GetID()) + "\"";
}
```
- **`Tool_DestroyEntity(paramsJson)`**: Deletes an entity from active scene by integer ID.
```cpp
std::string MCPPlugin::Tool_DestroyEntity(const std::string &paramsJson) {
    int64_t id = ExtractInt(paramsJson, "id");
    scene->DestroyEntity(Entity((EntityID)id, &manager));
    return "\"Destroyed entity with ID: " + std::to_string(id) + "\"";
}
```
- **`Tool_SelectEntity(paramsJson)`**: Updates selected entity in `EditorLayer` context.
- **`Tool_SetEntityProperties(paramsJson)`**: Deserializes JSON property maps and applies values to entity components.
- **`Tool_AddComponent(paramsJson)`**: Dynamically registers and attaches engine component types to an entity.

### 3. Editor Control & Viewport
- **`Tool_GetEditorModes()` & `Tool_SetEditorMode(paramsJson)`**: Queries or switches `EditorLayer::GetGlobalActiveMode()`.
```cpp
std::string MCPPlugin::Tool_SetEditorMode(const std::string &paramsJson) {
    std::string modeName = ExtractString(paramsJson, "mode");
    EditorLayer::SetGlobalActiveMode(modeName);
    return "\"Switched editor mode to " + modeName + "\"";
}
```
- **`Tool_GetViewportScreenshot()`**: Renders current viewport framebuffer, saves to temp disk file, and returns path.
- **`Tool_DeleteScreenshot()`**: Removes temporary viewport screenshot file after AI inspection.
- **`Tool_SendEditorInput(paramsJson)`**: Constructs and posts synthetic key/mouse events to engine event loop.

### 4. Asset & Directory Operations
- **`Tool_CreateSprite(paramsJson)`**: Creates `.tesprite` asset files inside `Assets/` directory.
- **`Tool_CreateDirectory(paramsJson)`**: Invokes `std::filesystem::create_directories` relative to project assets.
- **`Tool_DeletePath(paramsJson)`**: Invokes `std::filesystem::remove_all` on target path.

## Threading & Safety Model

- **Background Server Thread**: `ServerThreadMain()` runs on a dedicated thread spawned in `OnLoad()`.
- **Connection Isolation**: Incoming HTTP connections are handed off to detached worker threads (`HandleConnection`) so long-lived SSE connections do not block short-lived POST requests.
- **SSE Synchronization**: SSE client connections are stored in `m_SSEClients` protected by `m_SSEMutex`. Dead sockets are periodically pruned in `CleanupSSEClients()`.

