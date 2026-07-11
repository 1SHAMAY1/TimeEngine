// MCPPlugin.cpp — HTTP/SSE MCP server (MCP spec 2024-11-05)
//
// Endpoints:
//   GET  /sse          — SSE stream; server sends "endpoint" event pointing at /message
//   POST /message      — Client posts JSON-RPC 2.0 requests; server sends SSE response
//   GET  /health       — Simple 200 OK health check
//
// Connection flow:
//   1. AI client opens GET /sse  → receives "event: endpoint\ndata: /message\n\n"
//   2. AI client POSTs JSON-RPC to /message
//   3. Plugin dispatches tool, sends "event: message\ndata: <json>\n\n" back on SSE stream

#include "MCPPlugin.hpp"
#include "Core/Application.h"
#include "Core/Asset/AssetManager.hpp"
#include "Core/Events/KeyEvent.h"
#include "Core/Events/MouseEvent.h"
#include "Core/Log.h"
#include "Core/Project/Project.hpp"
#include "Core/Scene/ComponentRegistry.hpp"
#include "Core/Scene/Scene.hpp"
#include "Core/Scene/TagComponent.hpp"
#include "Core/Scene/TransformComponent.hpp"
#include "Editor/EditorMode.hpp"
#include "Layers/EditorLayer.hpp"
#include "Renderer/RenderCommand.hpp"

#ifdef TE_PLATFORM_WINDOWS
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")
#endif

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

namespace TE
{

// ---------------------------------------------------------------------------
// Plugin entry points
// ---------------------------------------------------------------------------

TE_PLUGIN_EXPORT IPlugin *CreatePluginInstance() { return new MCPPlugin(); }

TE_PLUGIN_EXPORT void DestroyPluginInstance(IPlugin *plugin) { delete plugin; }

// ---------------------------------------------------------------------------
// Lifecycle
// ---------------------------------------------------------------------------

void MCPPlugin::OnLoad()
{
    TE_CORE_INFO("[MCPPlugin] Starting HTTP/SSE MCP server...");
    m_Running = true;
    m_ServerThread = std::thread(&MCPPlugin::ServerThreadMain, this);
}

void MCPPlugin::OnUnload()
{
    TE_CORE_INFO("[MCPPlugin] Shutting down MCP server...");
    m_Running = false;

#ifdef TE_PLATFORM_WINDOWS
    if (m_ListenSocket != 0)
    {
        closesocket((SOCKET)m_ListenSocket);
        m_ListenSocket = 0;
    }
    // Close all SSE clients
    {
        std::lock_guard<std::mutex> lock(m_SSEMutex);
        for (auto &c : m_SSEClients)
        {
            if (c.Active)
                closesocket((SOCKET)c.Socket);
        }
        m_SSEClients.clear();
    }
#endif

    if (m_ServerThread.joinable())
        m_ServerThread.join();

    TE_CORE_INFO("[MCPPlugin] MCP server stopped.");
}

// ---------------------------------------------------------------------------
// Server accept loop
// ---------------------------------------------------------------------------

void MCPPlugin::ServerThreadMain()
{
#ifdef TE_PLATFORM_WINDOWS
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        TE_CORE_ERROR("[MCPPlugin] WSAStartup failed.");
        return;
    }

    SOCKET listenSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listenSock == INVALID_SOCKET)
    {
        TE_CORE_ERROR("[MCPPlugin] socket() failed.");
        WSACleanup();
        return;
    }

    // Allow port reuse to avoid TIME_WAIT issues on restart
    int yes = 1;
    setsockopt(listenSock, SOL_SOCKET, SO_REUSEADDR, (const char *)&yes, sizeof(yes));

    // Non-blocking so we can check m_Running
    u_long nb = 1;
    ioctlsocket(listenSock, FIONBIO, &nb);

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK); // 127.0.0.1 only
    addr.sin_port = htons(3000);

    if (bind(listenSock, (sockaddr *)&addr, sizeof(addr)) == SOCKET_ERROR)
    {
        TE_CORE_ERROR("[MCPPlugin] bind() failed: ", WSAGetLastError());
        closesocket(listenSock);
        WSACleanup();
        return;
    }

    if (listen(listenSock, SOMAXCONN) == SOCKET_ERROR)
    {
        TE_CORE_ERROR("[MCPPlugin] listen() failed: ", WSAGetLastError());
        closesocket(listenSock);
        WSACleanup();
        return;
    }

    m_ListenSocket = (uintptr_t)listenSock;
    TE_CORE_INFO("[MCPPlugin] MCP HTTP/SSE server listening on http://127.0.0.1:3000");

    while (m_Running)
    {
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(listenSock, &fds);
        timeval tv{0, 100000}; // 100 ms

        if (select(0, &fds, nullptr, nullptr, &tv) > 0)
        {
            SOCKET client = accept(listenSock, nullptr, nullptr);
            if (client != INVALID_SOCKET)
            {
                // Handle each connection on its own thread so SSE streams don't block
                uintptr_t clientHandle = (uintptr_t)client;
                std::thread([this, clientHandle]() { HandleConnection(clientHandle); }).detach();
            }
        }

        // Periodic cleanup of dead SSE clients
        CleanupSSEClients();
    }

    closesocket(listenSock);
    WSACleanup();
#endif
}

// ---------------------------------------------------------------------------
// HTTP connection handler
// ---------------------------------------------------------------------------

void MCPPlugin::HandleConnection(uintptr_t clientSocket)
{
#ifdef TE_PLATFORM_WINDOWS
    u_long mode = 0; // 0 to disable non-blocking (sets to blocking)
    ioctlsocket((SOCKET)clientSocket, FIONBIO, &mode);
#endif

    std::string req = ReadHttpRequest(clientSocket);
    if (req.empty())
    {
#ifdef TE_PLATFORM_WINDOWS
        closesocket((SOCKET)clientSocket);
#endif
        return;
    }

    // Parse request line: METHOD /path HTTP/1.x
    std::string method, path;
    {
        std::istringstream ss(req);
        ss >> method >> path;
    }

    // Normalise method to uppercase
    for (auto &c : method)
        c = (char)toupper((unsigned char)c);

    // -----------------------------------------------------------------------
    // GET /health
    // -----------------------------------------------------------------------
    if (method == "GET" && path == "/health")
    {
        SendHttpResponse(clientSocket, 200, "text/plain", "OK");
        return;
    }

    // -----------------------------------------------------------------------
    // GET /sse  — open SSE stream
    // -----------------------------------------------------------------------
    if (method == "GET" && path == "/sse")
    {
        // Send SSE headers — keep connection alive
        std::string headers = "HTTP/1.1 200 OK\r\n"
                              "Content-Type: text/event-stream\r\n"
                              "Cache-Control: no-cache\r\n"
                              "Connection: keep-alive\r\n"
                              "Access-Control-Allow-Origin: *\r\n"
                              "\r\n";
#ifdef TE_PLATFORM_WINDOWS
        send((SOCKET)clientSocket, headers.c_str(), (int)headers.size(), 0);
#endif

        // Tell the client where to POST messages (MCP "endpoint" event)
        SendSSEEvent(clientSocket, "endpoint", "http://127.0.0.1:3000/message");

        // Register as an active SSE client
        {
            std::lock_guard<std::mutex> lock(m_SSEMutex);
            m_SSEClients.push_back({clientSocket, true});
        }

        // Keep this thread alive; the socket stays open until disconnected or shutdown
        while (m_Running)
        {
            // Send a keep-alive comment every 15 s
            std::string ping = ": ping\n\n";
#ifdef TE_PLATFORM_WINDOWS
            int sent = send((SOCKET)clientSocket, ping.c_str(), (int)ping.size(), 0);
            if (sent == SOCKET_ERROR)
                break; // Client disconnected
#endif
            // Sleep 15 s in 100 ms increments so we respect m_Running
            for (int i = 0; i < 150 && m_Running; ++i)
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        // Mark client as inactive
        {
            std::lock_guard<std::mutex> lock(m_SSEMutex);
            for (auto &c : m_SSEClients)
            {
                if (c.Socket == clientSocket)
                    c.Active = false;
            }
        }

#ifdef TE_PLATFORM_WINDOWS
        closesocket((SOCKET)clientSocket);
#endif
        return;
    }

    // -----------------------------------------------------------------------
    // OPTIONS /message  — CORS preflight
    // -----------------------------------------------------------------------
    if (method == "OPTIONS")
    {
        std::string resp = "HTTP/1.1 204 No Content\r\n"
                           "Access-Control-Allow-Origin: *\r\n"
                           "Access-Control-Allow-Methods: POST, GET, OPTIONS\r\n"
                           "Access-Control-Allow-Headers: Content-Type\r\n"
                           "Content-Length: 0\r\n"
                           "\r\n";
#ifdef TE_PLATFORM_WINDOWS
        send((SOCKET)clientSocket, resp.c_str(), (int)resp.size(), 0);
        closesocket((SOCKET)clientSocket);
#endif
        return;
    }

    // -----------------------------------------------------------------------
    // POST /message  — JSON-RPC 2.0 from MCP client
    // -----------------------------------------------------------------------
    if (method == "POST" && path == "/message")
    {
        // Extract body (after \r\n\r\n)
        std::string body;
        size_t headerEnd = req.find("\r\n\r\n");
        if (headerEnd != std::string::npos)
            body = req.substr(headerEnd + 4);

        if (body.empty())
        {
            SendHttpResponse(
                clientSocket, 400, "application/json",
                "{\"jsonrpc\":\"2.0\",\"error\":{\"code\":-32700,\"message\":\"Empty body\"},\"id\":null}");
            return;
        }

        // Extract JSON-RPC fields
        std::string rpcMethod = ExtractString(body, "method");
        std::string idStr = ExtractString(body, "id");
        // id can be number — fall back to raw extraction
        if (idStr.empty())
        {
            size_t idPos = body.find("\"id\"");
            if (idPos != std::string::npos)
            {
                size_t colon = body.find(':', idPos);
                if (colon != std::string::npos)
                {
                    size_t valStart = body.find_first_not_of(" \t\r\n", colon + 1);
                    size_t valEnd = body.find_first_of(",}\r\n", valStart);
                    if (valStart != std::string::npos)
                        idStr = body.substr(valStart, valEnd - valStart);
                }
            }
        }
        if (idStr.empty())
            idStr = "null";

        // Strip surrounding whitespace from idStr
        while (!idStr.empty() && isspace((unsigned char)idStr.front()))
            idStr.erase(idStr.begin());
        while (!idStr.empty() && isspace((unsigned char)idStr.back()))
            idStr.pop_back();

        std::string responseJson;

        // ------------------------------------------------------------------
        // MCP: initialize
        // ------------------------------------------------------------------
        if (rpcMethod == "initialize")
        {
            responseJson = "{\"jsonrpc\":\"2.0\","
                           "\"result\":{"
                           "\"protocolVersion\":\"2024-11-05\","
                           "\"capabilities\":{\"tools\":{\"listChanged\":false}},"
                           "\"serverInfo\":{\"name\":\"TimeEngine\",\"version\":\"1.0.0\"}"
                           "},"
                           "\"id\":" +
                           idStr + "}";
        }
        // ------------------------------------------------------------------
        // MCP: notifications/initialized  (no response needed)
        // ------------------------------------------------------------------
        else if (rpcMethod == "notifications/initialized")
        {
            // Just acknowledge with 200, no JSON-RPC body required
            SendHttpResponse(clientSocket, 200, "application/json", "{}");
            BroadcastSSE("message", "{}");
            return;
        }
        // ------------------------------------------------------------------
        // MCP: tools/list
        // ------------------------------------------------------------------
        else if (rpcMethod == "tools/list")
        {
            responseJson =
                "{\"jsonrpc\":\"2.0\","
                "\"result\":{\"tools\":["
                "{"
                "\"name\":\"get_engine_info\","
                "\"description\":\"Retrieve TimeEngine version, active project name, and current configuration.\","
                "\"inputSchema\":{\"type\":\"object\",\"properties\":{}}"
                "},"
                "{"
                "\"name\":\"get_scene_hierarchy\","
                "\"description\":\"Get a list of all entities in the active scene with their IDs and tags.\","
                "\"inputSchema\":{\"type\":\"object\",\"properties\":{}}"
                "},"
                "{"
                "\"name\":\"create_entity\","
                "\"description\":\"Create a new named entity in the active scene.\","
                "\"inputSchema\":{"
                "\"type\":\"object\","
                "\"properties\":{"
                "\"name\":{\"type\":\"string\",\"description\":\"Name/tag for the new entity\"}"
                "},"
                "\"required\":[\"name\"]"
                "}"
                "},"
                "{"
                "\"name\":\"destroy_entity\","
                "\"description\":\"Destroy an entity by its integer ID.\","
                "\"inputSchema\":{"
                "\"type\":\"object\","
                "\"properties\":{"
                "\"id\":{\"type\":\"integer\",\"description\":\"Entity ID to destroy\"}"
                "},"
                "\"required\":[\"id\"]"
                "}"
                "},"
                "{"
                "\"name\":\"create_sprite\","
                "\"description\":\"Create a .tesprite asset file at the given relative path inside the project assets "
                "folder.\","
                "\"inputSchema\":{"
                "\"type\":\"object\","
                "\"properties\":{"
                "\"name\":{\"type\":\"string\",\"description\":\"Sprite asset name (no extension)\"},"
                "\"path\":{\"type\":\"string\",\"description\":\"Relative folder path inside Assets/\"}"
                "},"
                "\"required\":[\"name\",\"path\"]"
                "}"
                "},"
                "{"
                "\"name\":\"create_directory\","
                "\"description\":\"Create a directory relative to the active project's Assets folder.\","
                "\"inputSchema\":{"
                "\"type\":\"object\","
                "\"properties\":{"
                "\"path\":{\"type\":\"string\",\"description\":\"Relative directory path to create\"}"
                "},"
                "\"required\":[\"path\"]"
                "}"
                "},"
                "{"
                "\"name\":\"delete_file_or_directory\","
                "\"description\":\"Delete a file or directory relative to the project Assets folder.\","
                "\"inputSchema\":{"
                "\"type\":\"object\","
                "\"properties\":{"
                "\"path\":{\"type\":\"string\",\"description\":\"Relative path to delete\"}"
                "},"
                "\"required\":[\"path\"]"
                "}"
                "},"
                "{"
                "\"name\":\"get_editor_modes\","
                "\"description\":\"Get list of registered editor modes and highlight the active mode.\","
                "\"inputSchema\":{\"type\":\"object\",\"properties\":{}}"
                "},"
                "{"
                "\"name\":\"set_editor_mode\","
                "\"description\":\"Switch the current active editor mode by name.\","
                "\"inputSchema\":{"
                "\"type\":\"object\","
                "\"properties\":{"
                "\"mode\":{\"type\":\"string\",\"description\":\"The editor mode name to switch to\"}"
                "},"
                "\"required\":[\"mode\"]"
                "}"
                "},"
                "{"
                "\"name\":\"get_viewport_screenshot\","
                "\"description\":\"Take screenshot of editor viewport, save to temp file, return absolute path, and "
                "clean up.\","
                "\"inputSchema\":{\"type\":\"object\",\"properties\":{}}"
                "},"
                "{"
                "\"name\":\"delete_screenshot\","
                "\"description\":\"Delete the temporary screenshot file from disk once processed by client.\","
                "\"inputSchema\":{\"type\":\"object\",\"properties\":{}}"
                "},"
                "{"
                "\"name\":\"send_editor_input\","
                "\"description\":\"Simulate editor input (e.g. key_press, mouse_press).\","
                "\"inputSchema\":{"
                "\"type\":\"object\","
                "\"properties\":{"
                "\"type\":{\"type\":\"string\",\"description\":\"Input event type: key_press, key_release, "
                "mouse_press, mouse_release\"},"
                "\"code\":{\"type\":\"integer\",\"description\":\"The integer KeyCode or MouseCode\"}"
                "},"
                "\"required\":[\"type\",\"code\"]"
                "}"
                "},"
                "{"
                "\"name\":\"select_entity\","
                "\"description\":\"Select a specific entity in the editor by ID.\","
                "\"inputSchema\":{"
                "\"type\":\"object\","
                "\"properties\":{"
                "\"id\":{\"type\":\"integer\",\"description\":\"The entity ID to select\"}"
                "},"
                "\"required\":[\"id\"]"
                "}"
                "},"
                "{"
                "\"name\":\"set_entity_properties\","
                "\"description\":\"Set properties of an entity's components using JSON serialized key-values.\","
                "\"inputSchema\":{"
                "\"type\":\"object\","
                "\"properties\":{"
                "\"id\":{\"type\":\"integer\",\"description\":\"Entity ID\"},"
                "\"properties\":{\"type\":\"object\",\"description\":\"JSON object mapping component names to "
                "key-value variables\"}"
                "},"
                "\"required\":[\"id\",\"properties\"]"
                "}"
                "}"
                "]},"
                "\"id\":" +
                idStr + "}";
        }
        // ------------------------------------------------------------------
        // MCP: tools/call
        // ------------------------------------------------------------------
        else if (rpcMethod == "tools/call")
        {
            std::string toolName = ExtractString(body, "name");
            std::string paramsJson = ExtractObject(body, "arguments");
            if (paramsJson.empty())
                paramsJson = "{}";

            std::string toolResult = DispatchToolCall(toolName, paramsJson);

            responseJson = "{\"jsonrpc\":\"2.0\","
                           "\"result\":{\"content\":[{\"type\":\"text\",\"text\":" +
                           toolResult +
                           "}]},"
                           "\"id\":" +
                           idStr + "}";
        }
        // ------------------------------------------------------------------
        // Unknown method
        // ------------------------------------------------------------------
        else
        {
            responseJson = "{\"jsonrpc\":\"2.0\","
                           "\"error\":{\"code\":-32601,\"message\":\"Method not found: " +
                           rpcMethod +
                           "\"},"
                           "\"id\":" +
                           idStr + "}";
        }

        // Respond with 200 OK containing the JSON-RPC response body
        SendHttpResponse(clientSocket, 200, "application/json", responseJson);

        // Also push the result over SSE to any connected stream clients
        BroadcastSSE("message", responseJson);
        return;
    }

    // -----------------------------------------------------------------------
    // 404 fallback
    // -----------------------------------------------------------------------
    SendHttpResponse(clientSocket, 404, "text/plain", "Not Found");
}

// ---------------------------------------------------------------------------
// Tool dispatch
// ---------------------------------------------------------------------------

std::string MCPPlugin::DispatchToolCall(const std::string &toolName, const std::string &paramsJson)
{
    if (toolName == "get_engine_info")
        return Tool_GetEngineInfo();
    if (toolName == "get_scene_hierarchy")
        return Tool_GetSceneHierarchy();
    if (toolName == "create_entity")
        return Tool_CreateEntity(paramsJson);
    if (toolName == "destroy_entity")
        return Tool_DestroyEntity(paramsJson);
    if (toolName == "create_sprite")
        return Tool_CreateSprite(paramsJson);
    if (toolName == "create_directory")
        return Tool_CreateDirectory(paramsJson);
    if (toolName == "delete_file_or_directory")
        return Tool_DeletePath(paramsJson);

    // NEW Tools
    if (toolName == "get_editor_modes")
        return Tool_GetEditorModes();
    if (toolName == "set_editor_mode")
        return Tool_SetEditorMode(paramsJson);
    if (toolName == "get_viewport_screenshot")
        return Tool_GetViewportScreenshot();
    if (toolName == "send_editor_input")
        return Tool_SendEditorInput(paramsJson);
    if (toolName == "select_entity")
        return Tool_SelectEntity(paramsJson);
    if (toolName == "set_entity_properties")
        return Tool_SetEntityProperties(paramsJson);
    if (toolName == "add_component")
        return Tool_AddComponent(paramsJson);
    if (toolName == "delete_screenshot")
        return Tool_DeleteScreenshot();

    return "\"Error: Unknown tool '" + toolName + "'\"";
}

// ---------------------------------------------------------------------------
// Tool implementations
// ---------------------------------------------------------------------------

std::string MCPPlugin::Tool_GetEngineInfo()
{
    std::string projectName = "None";
    std::string projectPath = "N/A";
    if (Project::GetActive())
    {
        projectName = Project::GetActiveConfig().Name;
        projectPath = Project::GetProjectDirectory().string();
    }
    std::string info = "TimeEngine v1.0 (C++20) | Project: " + projectName + " | Path: " + projectPath;
    return "\"" + info + "\"";
}

std::string MCPPlugin::Tool_GetSceneHierarchy()
{
    const auto &layers = Application::Get().GetLayerStack();
    for (Layer *layer : layers)
    {
        if (!layer || layer->GetName() != "EditorLayer")
            continue;

        EditorLayer *editorLayer = static_cast<EditorLayer *>(layer);
        auto scene = editorLayer->GetActiveScene();
        if (!scene)
            break;

        std::ostringstream ss;
        ss << "Scene: " << scene->GetName() << "\\nEntities:";
        auto &manager = scene->GetEntityManager();
        for (EntityID id : manager.GetAliveEntities())
        {
            Entity entity(id, &manager);
            std::string name = "Entity";
            if (entity.HasComponent<TagComponent>())
                name = entity.GetComponent<TagComponent>()->Tag;
            ss << "\\n- [" << id << "] " << name;
        }
        return "\"" + ss.str() + "\"";
    }
    return "\"No active scene\"";
}

std::string MCPPlugin::Tool_CreateEntity(const std::string &paramsJson)
{
    std::string name = ExtractString(paramsJson, "name");
    if (name.empty())
        name = "NewEntity";

    const auto &layers = Application::Get().GetLayerStack();
    for (Layer *layer : layers)
    {
        if (!layer || layer->GetName() != "EditorLayer")
            continue;

        EditorLayer *editorLayer = static_cast<EditorLayer *>(layer);
        auto scene = editorLayer->GetActiveScene();
        if (!scene)
            break;

        Entity e = scene->CreateEntity(name);
        return "\"Created entity '" + name + "' with ID: " + std::to_string(e.GetID()) + "\"";
    }
    return "\"Error: No active scene\"";
}

std::string MCPPlugin::Tool_DestroyEntity(const std::string &paramsJson)
{
    int64_t id = ExtractInt(paramsJson, "id");
    if (id < 0)
        return "\"Error: Invalid or missing entity ID\"";

    const auto &layers = Application::Get().GetLayerStack();
    for (Layer *layer : layers)
    {
        if (!layer || layer->GetName() != "EditorLayer")
            continue;

        EditorLayer *editorLayer = static_cast<EditorLayer *>(layer);
        auto scene = editorLayer->GetActiveScene();
        if (!scene)
            break;

        auto &manager = scene->GetEntityManager();
        if (!manager.IsValid((EntityID)id))
            return "\"Error: Entity " + std::to_string(id) + " does not exist\"";

        Entity e((EntityID)id, &manager);
        scene->DestroyEntity(e);
        return "\"Destroyed entity with ID: " + std::to_string(id) + "\"";
    }
    return "\"Error: No active scene\"";
}

std::string MCPPlugin::Tool_CreateSprite(const std::string &paramsJson)
{
    std::string name = ExtractString(paramsJson, "name");
    std::string relPath = ExtractString(paramsJson, "path");

    if (!Project::GetActive())
        return "\"Error: No active project\"";
    if (name.empty() || relPath.empty())
        return "\"Error: 'name' and 'path' are required\"";

    std::filesystem::path fullPath = Project::GetAssetDirectory() / relPath / (name + ".tesprite");
    std::filesystem::create_directories(fullPath.parent_path());

    std::ofstream file(fullPath);
    if (!file.is_open())
        return "\"Error: Cannot write to " + fullPath.string() + "\"";

    file << "Sprite: " << name << "\n";
    file << "Texture: 0\n";
    file << "UVs: 0 0 1 1\n";
    file.close();

    return "\"Created sprite at: " + fullPath.string() + "\"";
}

std::string MCPPlugin::Tool_CreateDirectory(const std::string &paramsJson)
{
    std::string relPath = ExtractString(paramsJson, "path");

    if (!Project::GetActive())
        return "\"Error: No active project\"";
    if (relPath.empty())
        return "\"Error: 'path' is required\"";

    std::filesystem::path fullPath = Project::GetAssetDirectory() / relPath;
    std::error_code ec;
    if (std::filesystem::create_directories(fullPath, ec) || std::filesystem::exists(fullPath))
        return "\"Directory ready: " + fullPath.string() + "\"";

    return "\"Error: " + ec.message() + "\"";
}

std::string MCPPlugin::Tool_DeletePath(const std::string &paramsJson)
{
    std::string relPath = ExtractString(paramsJson, "path");

    if (!Project::GetActive())
        return "\"Error: No active project\"";
    if (relPath.empty())
        return "\"Error: 'path' is required\"";

    std::filesystem::path fullPath = Project::GetAssetDirectory() / relPath;
    if (!std::filesystem::exists(fullPath))
        return "\"Error: Path not found: " + fullPath.string() + "\"";

    std::error_code ec;
    uintmax_t count = std::filesystem::remove_all(fullPath, ec);
    if (ec)
        return "\"Error: " + ec.message() + "\"";

    return "\"Deleted " + std::to_string(count) + " item(s) at: " + fullPath.string() + "\"";
}

// ---------------------------------------------------------------------------
// NEW MCP Tool Implementations
// ---------------------------------------------------------------------------

std::string MCPPlugin::Tool_GetEditorModes()
{
    std::ostringstream ss;
    const auto &modes = EditorLayer::GetGlobalModes();
    EditorMode *active = EditorLayer::GetGlobalActiveMode();

    ss << "Available Editor Modes: [";
    for (size_t i = 0; i < modes.size(); ++i)
    {
        ss << modes[i]->GetName();
        if (i < modes.size() - 1)
            ss << ", ";
    }
    ss << "] | Active: " << (active ? active->GetName() : "None");
    return "\"" + ss.str() + "\"";
}

std::string MCPPlugin::Tool_SetEditorMode(const std::string &paramsJson)
{
    std::string modeName = ExtractString(paramsJson, "mode");
    if (modeName.empty())
        return "\"Error: 'mode' name parameter is required\"";

    const auto &modes = EditorLayer::GetGlobalModes();
    bool found = false;
    for (const auto &m : modes)
    {
        if (m->GetName() == modeName)
        {
            found = true;
            break;
        }
    }

    if (!found)
        return "\"Error: Mode '" + modeName + "' not found\"";

    EditorLayer::SetGlobalActiveMode(modeName);
    return "\"Switched editor mode to " + modeName + "\"";
}

std::string MCPPlugin::Tool_GetViewportScreenshot()
{
    const auto &layers = Application::Get().GetLayerStack();
    for (Layer *layer : layers)
    {
        if (!layer || layer->GetName() != "EditorLayer")
            continue;

        EditorLayer *editorLayer = static_cast<EditorLayer *>(layer);

        // Grab framebuffer spec using reflection metadata or direct access to framebuffer
        // We know from EditorLayer.hpp that m_Framebuffer is of type std::shared_ptr<Framebuffer>
        // and we have its pointer or we can read from standard RenderCommand screen size.
        // We will read pixels from active bound framebuffer target.
        uint32_t width = Application::Get().GetWindow().GetWidth();
        uint32_t height = Application::Get().GetWindow().GetHeight();

        std::vector<uint32_t> pixels(width * height);
        RenderCommand::ReadPixelsRGBA(0, 0, width, height, pixels.data());

        // Flip image Y coordinate since OpenGL reads from bottom-left
        std::vector<uint32_t> flipped(width * height);
        for (uint32_t y = 0; y < height; y++)
        {
            memcpy(&flipped[y * width], &pixels[(height - 1 - y) * width], width * 4);
        }

        // Export screenshot file
        std::filesystem::path screenshotPath = std::filesystem::current_path() / "temp_viewport_capture.png";

        // Save the PNG using built-in engine asset utilities
        AssetManager::ExportImagePNG(screenshotPath.string(), width, height, 4, flipped.data());

        return "\"Screenshot taken and saved temporarily to: " + screenshotPath.string() + "\"";
    }

    return "\"Error: EditorLayer viewport not found\"";
}

std::string MCPPlugin::Tool_DeleteScreenshot()
{
    std::filesystem::path screenshotPath = std::filesystem::current_path() / "temp_viewport_capture.png";
    std::error_code ec;
    if (std::filesystem::exists(screenshotPath))
    {
        std::filesystem::remove(screenshotPath, ec);
        return "\"Screenshot deleted successfully\"";
    }
    return "\"Error: No active screenshot found to delete\"";
}

std::string MCPPlugin::Tool_SendEditorInput(const std::string &paramsJson)
{
    std::string type = ExtractString(paramsJson, "type");
    int64_t code = ExtractInt(paramsJson, "code");

    if (type.empty() || code < 0)
        return "\"Error: 'type' (string) and 'code' (integer) are required\"";

    if (type == "key_press")
    {
        KeyPressedEvent event((KeyCode)code, false);
        Application::Get().OnUpdate(); // Flush frame ticks
        // Simulate OnEvent callback propagation
        const auto &layers = Application::Get().GetLayerStack();
        for (auto it = layers.end(); it != layers.begin();)
        {
            (*--it)->OnEvent(event);
        }
        return "\"Simulated key_press: " + std::to_string(code) + "\"";
    }
    else if (type == "key_release")
    {
        KeyReleasedEvent event((KeyCode)code);
        const auto &layers = Application::Get().GetLayerStack();
        for (auto it = layers.end(); it != layers.begin();)
        {
            (*--it)->OnEvent(event);
        }
        return "\"Simulated key_release: " + std::to_string(code) + "\"";
    }
    else if (type == "mouse_press")
    {
        MouseButtonPressedEvent event((MouseCode)code);
        const auto &layers = Application::Get().GetLayerStack();
        for (auto it = layers.end(); it != layers.begin();)
        {
            (*--it)->OnEvent(event);
        }
        return "\"Simulated mouse_press: " + std::to_string(code) + "\"";
    }
    else if (type == "mouse_release")
    {
        MouseButtonReleasedEvent event((MouseCode)code);
        const auto &layers = Application::Get().GetLayerStack();
        for (auto it = layers.end(); it != layers.begin();)
        {
            (*--it)->OnEvent(event);
        }
        return "\"Simulated mouse_release: " + std::to_string(code) + "\"";
    }

    return "\"Error: Invalid input type. Valid: key_press, key_release, mouse_press, mouse_release\"";
}

std::string MCPPlugin::Tool_SelectEntity(const std::string &paramsJson)
{
    int64_t entityId = ExtractInt(paramsJson, "id");
    if (entityId < 0)
        return "\"Error: Valid integer 'id' parameter is required\"";

    const auto &layers = Application::Get().GetLayerStack();
    for (Layer *layer : layers)
    {
        if (!layer || layer->GetName() != "EditorLayer")
            continue;

        EditorLayer *editorLayer = static_cast<EditorLayer *>(layer);
        auto scene = editorLayer->GetActiveScene();
        if (!scene)
            break;

        auto &manager = scene->GetEntityManager();
        if (!manager.IsValid((EntityID)entityId))
            return "\"Error: Entity ID " + std::to_string(entityId) + " is not valid in active scene\"";

        Entity entity((EntityID)entityId, &manager);

        // Use reflection method or force selection update inside EditorLayer
        // We know from EditorLayer.hpp: SelectEntity(Entity entity, bool multiSelect = false, bool toggle = false)
        // Let's call it via EditorLayer pointer! (It's marked TE_API / public in EditorLayer.hpp)
        editorLayer->ClearSelection();
        editorLayer->SelectEntity(entity, false, false);
        return "\"Selected entity with ID: " + std::to_string(entityId) + "\"";
    }

    return "\"Error: EditorLayer or active scene not found\"";
}

std::string MCPPlugin::Tool_SetEntityProperties(const std::string &paramsJson)
{
    int64_t entityId = ExtractInt(paramsJson, "id");
    std::string propObj = ExtractObject(paramsJson, "properties");

    if (entityId < 0 || propObj.empty())
        return "\"Error: 'id' (integer) and 'properties' (object) parameters are required\"";

    const auto &layers = Application::Get().GetLayerStack();
    for (Layer *layer : layers)
    {
        if (!layer || layer->GetName() != "EditorLayer")
            continue;

        EditorLayer *editorLayer = static_cast<EditorLayer *>(layer);
        auto scene = editorLayer->GetActiveScene();
        if (!scene)
            break;

        auto &manager = scene->GetEntityManager();
        if (!manager.IsValid((EntityID)entityId))
            return "\"Error: Entity ID " + std::to_string(entityId) + " not found\"";

        // Clean escaped quotes \" to normal quotes " in the entire properties object
        size_t bPos = 0;
        while ((bPos = propObj.find("\\\"", bPos)) != std::string::npos)
        {
            propObj.replace(bPos, 2, "\"");
            bPos += 1;
        }

        int successfulUpdates = 0;
        size_t searchPos = 0;
        while (searchPos < propObj.length())
        {
            size_t keyStart = propObj.find_first_of("\"", searchPos);
            if (keyStart == std::string::npos)
                break;
            size_t keyEnd = propObj.find_first_of("\"", keyStart + 1);
            if (keyEnd == std::string::npos)
                break;
            std::string compName = propObj.substr(keyStart + 1, keyEnd - keyStart - 1);

            size_t compValStart = propObj.find_first_of("{", keyEnd + 1);
            if (compValStart == std::string::npos)
                break;
            size_t compValEnd = compValStart;
            int braceCount = 1;
            while (braceCount > 0 && compValEnd < propObj.length() - 1)
            {
                compValEnd++;
                if (propObj[compValEnd] == '{')
                    braceCount++;
                if (propObj[compValEnd] == '}')
                {
                    braceCount--;
                    if (braceCount == 0)
                    {
                        compValEnd++;
                        break;
                    }
                }
            }

            std::string compPropsJson = propObj.substr(compValStart, compValEnd - compValStart);
            searchPos = compValEnd; // Update lookahead pointer

            TE_CORE_INFO("[MCPPlugin Diagnostic] compName: {0}, compPropsJson: {1}", compName, compPropsJson);

            // Update matching component instance on the target entity
            std::vector<TComponent *> allComponents = manager.GetAllComponents((EntityID)entityId);
            TE_CORE_INFO("[MCPPlugin Diagnostic] Entity ID: {0}, Total Components found: {1}", entityId,
                         allComponents.size());
            TComponent *targetComponent = nullptr;
            for (auto *c : allComponents)
            {
                if (c)
                {
                    const auto *meta = Scene::GetGlobalComponentRegistry().GetMetadata(std::type_index(typeid(*c)));
                    std::string cClassName = meta ? meta->ClassName : "";
                    if (cClassName.empty() && typeid(*c) == typeid(TransformComponent))
                    {
                        cClassName = "TransformComponent";
                    }

                    TE_CORE_INFO("[MCPPlugin Diagnostic] - cClassName: {0}, raw_name: {1}", cClassName,
                                 typeid(*c).name());

                    if (cClassName == compName)
                    {
                        targetComponent = c;
                        break;
                    }
                }
            }

            if (targetComponent)
            {
                if (compName == "TransformComponent")
                {
                    std::string posStr = ExtractString(compPropsJson, "Position");
                    std::string rotStr = ExtractString(compPropsJson, "Rotation");
                    std::string scaleStr = ExtractString(compPropsJson, "Scale");

                    TE_CORE_INFO("[MCPPlugin Diagnostic] TransformComponent: posStr={0}, rotStr={1}, scaleStr={2}",
                                 posStr, rotStr, scaleStr);

                    if (!posStr.empty())
                    {
                        std::stringstream ss(posStr);
                        ss >> targetComponent->Transform.Position.x >> targetComponent->Transform.Position.y >>
                            targetComponent->Transform.Position.z;
                        successfulUpdates++;
                    }
                    if (!rotStr.empty())
                    {
                        std::stringstream ss(rotStr);
                        ss >> targetComponent->Transform.Rotation.Pitch >> targetComponent->Transform.Rotation.Yaw >>
                            targetComponent->Transform.Rotation.Roll;
                        successfulUpdates++;
                    }
                    if (!scaleStr.empty())
                    {
                        std::stringstream ss(scaleStr);
                        ss >> targetComponent->Transform.Scale.Scale.x >> targetComponent->Transform.Scale.Scale.y >>
                            targetComponent->Transform.Scale.Scale.z;
                        successfulUpdates++;
                    }
                }
                else
                {
                    const auto *metadata = Scene::GetGlobalComponentRegistry().GetMetadata(compName);
                    if (metadata)
                    {
                        // Update matching properties using reflection registry
                        for (const auto &propMeta : metadata->Properties)
                        {
                            std::string propVal = ExtractString(compPropsJson, propMeta.Name);
                            if (propVal.empty())
                            {
                                size_t pPos = compPropsJson.find("\"" + propMeta.Name + "\"");
                                if (pPos != std::string::npos)
                                {
                                    size_t colon = compPropsJson.find(":", pPos);
                                    if (colon != std::string::npos)
                                    {
                                        size_t valStart = compPropsJson.find_first_not_of(" \t\r\n", colon + 1);
                                        size_t valEnd = compPropsJson.find_first_of(",}\r\n", valStart);
                                        if (valStart != std::string::npos)
                                            propVal = compPropsJson.substr(valStart, valEnd - valStart);
                                    }
                                }
                            }

                            TE_CORE_INFO("[MCPPlugin Diagnostic] Checking property: {0}, extracted value: {1}",
                                         propMeta.Name, propVal);

                            if (!propVal.empty() && propMeta.DeserializeFunc)
                            {
                                propMeta.DeserializeFunc(targetComponent, propVal);
                                successfulUpdates++;
                            }
                        }
                    }
                }
            }
        }

        return "\"Updated " + std::to_string(successfulUpdates) + " entity component properties successfully\"";
    }

    return "\"Error: Active EditorLayer or scene context missing\"";
}

std::string MCPPlugin::Tool_AddComponent(const std::string &paramsJson)
{
    int64_t entityId = ExtractInt(paramsJson, "id");
    std::string compType = ExtractString(paramsJson, "type");

    if (entityId < 0 || compType.empty())
        return "\"Error: 'id' (integer) and 'type' (string) parameters are required\"";

    const auto &layers = Application::Get().GetLayerStack();
    for (Layer *layer : layers)
    {
        if (!layer || layer->GetName() != "EditorLayer")
            continue;

        EditorLayer *editorLayer = static_cast<EditorLayer *>(layer);
        auto scene = editorLayer->GetActiveScene();
        if (!scene)
            break;

        auto &manager = scene->GetEntityManager();
        if (!manager.IsValid((EntityID)entityId))
            return "\"Error: Entity ID " + std::to_string(entityId) + " not found\"";

        const auto &factories = manager.GetRegisteredComponents();
        auto it = factories.find(compType);
        if (it == factories.end())
            return "\"Error: Component type '" + compType + "' not registered in EntityManager factories\"";

        // Call the factory to instantiate the component onto the entity
        TComponent *comp = it->second((EntityID)entityId);
        if (comp)
        {
            comp->OnInitialize();
            return "\"Added component '" + compType + "' to entity " + std::to_string(entityId) + "\"";
        }
        return "\"Error: Component factory failed to instantiate component\"";
    }

    return "\"Error: Active EditorLayer or scene context missing\"";
}

// ---------------------------------------------------------------------------
// HTTP / SSE helpers
// ---------------------------------------------------------------------------

std::string MCPPlugin::ReadHttpRequest(uintptr_t socket)
{
    std::string result;
    result.reserve(4096);

#ifdef TE_PLATFORM_WINDOWS
    DWORD timeout = 3000;
    setsockopt((SOCKET)socket, SOL_SOCKET, SO_RCVTIMEO, (const char *)&timeout, sizeof(timeout));

    char buf[4096];
    int bytes;

    // Phase 1: read until we have complete headers (\r\n\r\n)
    while ((bytes = recv((SOCKET)socket, buf, sizeof(buf) - 1, 0)) > 0)
    {
        buf[bytes] = '\0';
        result += buf;
        if (result.find("\r\n\r\n") != std::string::npos)
            break;
    }

    if (result.find("\r\n\r\n") == std::string::npos)
        return result; // headers never completed

    TE_CORE_INFO("[MCPPlugin Diagnostic] Raw Headers received:\n{0}", result);

    // Handle "Expect: 100-continue" — client waits for this before sending body
    bool hasExpect = (result.find("Expect: 100-continue") != std::string::npos ||
                      result.find("expect: 100-continue") != std::string::npos);
    if (hasExpect)
    {
        const char *cont = "HTTP/1.1 100 Continue\r\n\r\n";
        send((SOCKET)socket, cont, (int)strlen(cont), 0);
    }

    // Phase 2: read body according to Content-Length
    size_t clPos = result.find("Content-Length:");
    if (clPos == std::string::npos)
        return result; // no body (GET / OPTIONS)

    size_t clEnd = result.find("\r\n", clPos);
    int contentLength = 0;
    try
    {
        std::string clVal = result.substr(clPos + 15, clEnd - (clPos + 15));
        // Remove whitespace
        clVal.erase(std::remove_if(clVal.begin(), clVal.end(), ::isspace), clVal.end());
        contentLength = std::stoi(clVal);
    }
    catch (...)
    {
        return result;
    }

    size_t headerEnd = result.find("\r\n\r\n") + 4;
    int bodyReceived = (int)(result.size() - headerEnd);

    while (bodyReceived < contentLength)
    {
        bytes = recv((SOCKET)socket, buf, sizeof(buf) - 1, 0);
        if (bytes <= 0)
            break;
        buf[bytes] = '\0';
        result += buf;
        bodyReceived += bytes;
    }
#endif
    return result;
}

void MCPPlugin::SendHttpResponse(uintptr_t socket, int statusCode, const std::string &contentType,
                                 const std::string &body, bool keepAlive)
{
    std::string statusText = "OK";
    if (statusCode == 202)
        statusText = "Accepted";
    if (statusCode == 204)
        statusText = "No Content";
    if (statusCode == 400)
        statusText = "Bad Request";
    if (statusCode == 404)
        statusText = "Not Found";

    std::string response = "HTTP/1.1 " + std::to_string(statusCode) + " " + statusText +
                           "\r\n"
                           "Content-Type: " +
                           contentType +
                           "\r\n"
                           "Content-Length: " +
                           std::to_string(body.size()) +
                           "\r\n"
                           "Access-Control-Allow-Origin: *\r\n"
                           "Connection: " +
                           (keepAlive ? "keep-alive" : "close") +
                           "\r\n"
                           "\r\n" +
                           body;

#ifdef TE_PLATFORM_WINDOWS
    send((SOCKET)socket, response.c_str(), (int)response.size(), 0);
    if (!keepAlive)
        closesocket((SOCKET)socket);
#endif
}

void MCPPlugin::SendSSEEvent(uintptr_t socket, const std::string &eventName, const std::string &data)
{
    // SSE format: "event: <name>\ndata: <data>\n\n"
    std::string sseMsg = "event: " + eventName + "\ndata: " + data + "\n\n";
#ifdef TE_PLATFORM_WINDOWS
    send((SOCKET)socket, sseMsg.c_str(), (int)sseMsg.size(), 0);
#endif
}

void MCPPlugin::BroadcastSSE(const std::string &eventName, const std::string &data)
{
    std::lock_guard<std::mutex> lock(m_SSEMutex);
    for (auto &client : m_SSEClients)
    {
        if (!client.Active)
            continue;

        std::string sseMsg = "event: " + eventName + "\ndata: " + data + "\n\n";
#ifdef TE_PLATFORM_WINDOWS
        int sent = send((SOCKET)client.Socket, sseMsg.c_str(), (int)sseMsg.size(), 0);
        if (sent == SOCKET_ERROR)
            client.Active = false;
#endif
    }
}

void MCPPlugin::CleanupSSEClients()
{
    std::lock_guard<std::mutex> lock(m_SSEMutex);
    m_SSEClients.erase(
        std::remove_if(m_SSEClients.begin(), m_SSEClients.end(), [](const SSEClient &c) { return !c.Active; }),
        m_SSEClients.end());
}

// ---------------------------------------------------------------------------
// JSON helpers
// ---------------------------------------------------------------------------

std::string MCPPlugin::ExtractString(const std::string &json, const std::string &key)
{
    std::string search = "\"" + key + "\"";
    size_t keyPos = json.find(search);
    if (keyPos == std::string::npos)
        return "";

    size_t colon = json.find(':', keyPos + search.size());
    if (colon == std::string::npos)
        return "";

    size_t start = json.find('"', colon + 1);
    if (start == std::string::npos)
        return "";

    size_t end = start + 1;
    while (end < json.size())
    {
        if (json[end] == '\\')
        {
            end += 2;
            continue;
        } // skip escape
        if (json[end] == '"')
            break;
        end++;
    }

    return json.substr(start + 1, end - start - 1);
}

int64_t MCPPlugin::ExtractInt(const std::string &json, const std::string &key)
{
    std::string search = "\"" + key + "\"";
    size_t keyPos = json.find(search);
    if (keyPos == std::string::npos)
        return -1;

    size_t colon = json.find(':', keyPos + search.size());
    if (colon == std::string::npos)
        return -1;

    size_t valStart = json.find_first_of("-0123456789", colon + 1);
    if (valStart == std::string::npos)
        return -1;

    size_t valEnd = json.find_first_not_of("0123456789", valStart + 1);
    std::string numStr = json.substr(valStart, valEnd - valStart);
    try
    {
        return std::stoll(numStr);
    }
    catch (...)
    {
        return -1;
    }
}

// Extract a JSON object value: finds the '{' after the key and returns balanced braces
std::string MCPPlugin::ExtractObject(const std::string &json, const std::string &key)
{
    std::string search = "\"" + key + "\"";
    size_t keyPos = json.find(search);
    if (keyPos == std::string::npos)
        return "";

    size_t colon = json.find(':', keyPos + search.size());
    if (colon == std::string::npos)
        return "";

    size_t start = json.find('{', colon + 1);
    if (start == std::string::npos)
        return "";

    int depth = 0;
    size_t end = start;
    while (end < json.size())
    {
        if (json[end] == '{')
            depth++;
        else if (json[end] == '}')
        {
            depth--;
            if (depth == 0)
            {
                end++;
                break;
            }
        }
        end++;
    }

    return json.substr(start, end - start);
}

} // namespace TE
