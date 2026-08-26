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
#include "Core/Plugin/PluginManager.hpp"
#include "Core/Project/Project.hpp"
#include "Core/Scene/ComponentRegistry.hpp"
#include "Core/Scene/Scene.hpp"
#include "Core/Scene/TagComponent.hpp"
#include "Core/Scene/TransformComponent.hpp"
#include "Editor/AssetEditorRegistry.hpp"
#include "Editor/EditorMode.hpp"
#include "Layers/EditorLayer.hpp"
#include "MCPToolRegistry.hpp"
#include "Renderer/RenderCommand.hpp"

#ifdef TE_PLATFORM_WINDOWS
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")
#endif

#include "Utils/TEFileSystem.hpp"
#include <algorithm>
#include <cctype>
#include <fstream>
#include <sstream>
#include <thread>
#include <vector>

TE_REGISTER_PLUGIN(MCPPlugin);

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
        m_SSEClients.Empty();
    }
#endif

    if (m_ServerThread.joinable())
        m_ServerThread.join();

    TE_CORE_INFO("[MCPPlugin] MCP server stopped.");
}

void MCPPlugin::DrawThumbnail(TimeGUIDrawList &dl, const TEVector2 &min, const TEVector2 &max) const
{
    float w = max.x - min.x;
    float h = max.y - min.y;
    TEVector2 c = TEVector2(min.x + w * 0.5f, min.y + h * 0.5f);

    unsigned int bgCol = 0xFF0A1E30;
    unsigned int borderCol = 0xFF00D2D3;
    dl.AddRectFilled(min, max, bgCol, 6.0f);
    dl.AddRect(min, max, borderCol, 6.0f, 0, 1.0f);

    // Neural network peripheral nodes
    TEVector2 n1(c.x - 13.0f, c.y - 10.0f);
    TEVector2 n2(c.x + 13.0f, c.y - 10.0f);
    TEVector2 n3(c.x - 14.0f, c.y + 11.0f);
    TEVector2 n4(c.x + 14.0f, c.y + 11.0f);
    TEVector2 n5(c.x, c.y - 14.0f);

    // Circuit connections
    dl.AddLine(n1, c, 0xFF0984E3, 1.5f);
    dl.AddLine(n2, c, 0xFF0984E3, 1.5f);
    dl.AddLine(n3, c, 0xFF0984E3, 1.5f);
    dl.AddLine(n4, c, 0xFF0984E3, 1.5f);
    dl.AddLine(n5, c, 0xFF0984E3, 1.5f);
    dl.AddLine(n1, n3, 0x6600D2D3, 1.0f);
    dl.AddLine(n2, n4, 0x6600D2D3, 1.0f);

    // Peripheral node disks
    dl.AddCircleFilled(n1, 3.0f, 0xFF70A1FF);
    dl.AddCircleFilled(n2, 3.0f, 0xFF70A1FF);
    dl.AddCircleFilled(n3, 3.0f, 0xFF70A1FF);
    dl.AddCircleFilled(n4, 3.0f, 0xFF70A1FF);
    dl.AddCircleFilled(n5, 2.5f, 0xFF55EFC4);

    // Central diamond core
    TEVector2 corePts[4] = {TEVector2(c.x, c.y - 6.0f), TEVector2(c.x + 6.0f, c.y), TEVector2(c.x, c.y + 6.0f),
                            TEVector2(c.x - 6.0f, c.y)};
    dl.AddConvexPolyFilled(corePts, 4, 0xFF00D2D3);
    dl.AddCircleFilled(c, 2.5f, 0xFFFFFFFF);
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

    TEString req = ReadHttpRequest(clientSocket);
    if (req.empty())
    {
#ifdef TE_PLATFORM_WINDOWS
        closesocket((SOCKET)clientSocket);
#endif
        return;
    }

    // Parse request line: METHOD /path HTTP/1.x
    TEString method, path;
    {
        TEArray<TEString> parts = req.Split(' ');
        if (parts.Size() >= 2)
        {
            method = parts[0];
            path = parts[1];
        }
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
        TEString headers = "HTTP/1.1 200 OK\r\n"
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
            m_SSEClients.Add({clientSocket, true});
        }

        // Keep this thread alive; the socket stays open until disconnected or shutdown
        while (m_Running)
        {
            // Send a keep-alive comment every 15 s
            TEString ping = ": ping\n\n";
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
        TEString resp = "HTTP/1.1 204 No Content\r\n"
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
        TEString body;
        size_t headerEnd = req.find("\r\n\r\n");
        if (headerEnd != TEString::npos)
            body = req.substr(headerEnd + 4);

        if (body.empty())
        {
            SendHttpResponse(
                clientSocket, 400, "application/json",
                "{\"jsonrpc\":\"2.0\",\"error\":{\"code\":-32700,\"message\":\"Empty body\"},\"id\":null}");
            return;
        }

        // Extract JSON-RPC fields
        TEString rpcMethod = ExtractString(body, "method");
        TEString idStr = ExtractString(body, "id");
        // id can be number — fall back to raw extraction
        if (idStr.IsEmpty())
        {
            size_t idPos = body.Find("\"id\"");
            if (idPos != TEString::npos)
            {
                size_t colon = body.Find(":", ESearchCase::CaseSensitive, ESearchDir::FromStart, (int)idPos);
                if (colon != TEString::npos)
                {
                    size_t valStart = body.find_first_not_of(" \t\r\n", colon + 1);
                    size_t valEnd = TEString::npos;
                    if (valStart != TEString::npos)
                    {
                        for (size_t i = valStart; i < body.Len(); ++i)
                        {
                            char ch = body[i];
                            if (ch == ',' || ch == '}' || ch == '\r' || ch == '\n')
                            {
                                valEnd = i;
                                break;
                            }
                        }
                        idStr = body.Substr(valStart, valEnd != TEString::npos ? valEnd - valStart : TEString::npos);
                    }
                }
            }
        }
        if (idStr.IsEmpty())
            idStr = "null";

        // Strip surrounding whitespace from idStr
        idStr.TrimInline();

        TEString responseJson;

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
            std::ostringstream json;
            json << "{\"jsonrpc\":\"2.0\",\"id\":" << idStr << ",\"result\":{\"tools\":[";

            const auto &tools = MCPToolRegistry::GetTools();
            for (size_t i = 0; i < tools.size(); i++)
            {
                if (i > 0)
                    json << ",";
                json << "{"
                     << "\"name\":\"" << tools[i].Name << "\","
                     << "\"description\":\"" << tools[i].Description << "\","
                     << "\"inputSchema\":"
                     << (tools[i].InputSchemaJson.empty() ? "{\"type\":\"object\",\"properties\":{}}"
                                                          : tools[i].InputSchemaJson)
                     << "}";
            }

            json << "]}}";
            responseJson = json.str();
        }
        // ------------------------------------------------------------------
        // MCP: tools/call
        // ------------------------------------------------------------------
        else if (rpcMethod == "tools/call")
        {
            TEString toolName = ExtractString(body, "name");
            TEString paramsJson = ExtractObject(body, "arguments");

            TEString toolResult = DispatchToolCall(toolName, paramsJson);

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

TEString MCPPlugin::DispatchToolCall(const TEString &toolName, const TEString &paramsJson)
{
    TEString result;
    if (MCPToolRegistry::ExecuteTool(toolName, paramsJson, result))
    {
        return result;
    }

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
    if (toolName == "open_asset_editor")
        return Tool_OpenAssetEditor(paramsJson);

    return "\"Error: Unknown tool '" + toolName + "'\"";
}

// ---------------------------------------------------------------------------
// Tool implementations
// ---------------------------------------------------------------------------

TEString MCPPlugin::Tool_GetEngineInfo()
{
    TEString projectName = "None";
    TEString projectPath = "N/A";
    if (Project::GetActive())
    {
        projectName = Project::GetActiveConfig().Name;
        projectPath = Project::GetProjectDirectory();
    }
    TEString info = "TimeEngine v1.0 (C++20) | Project: " + projectName + " | Path: " + projectPath;
    return "\"" + info + "\"";
}

TEString MCPPlugin::Tool_GetSceneHierarchy()
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
            TEString name = "Entity";
            if (entity.HasComponent<TagComponent>())
                name = entity.GetComponent<TagComponent>()->Tag;
            ss << "\\n- [" << id << "] " << name;
        }
        return "\"" + ss.str() + "\"";
    }
    return "\"No active scene\"";
}

TEString MCPPlugin::Tool_CreateEntity(const TEString &paramsJson)
{
    TEString name = ExtractString(paramsJson, "name");
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
        return "\"Created entity '" + name + "' with ID: " + TEString::FromInt((int)e.GetID()) + "\"";
    }
    return "\"Error: No active scene\"";
}

TEString MCPPlugin::Tool_DestroyEntity(const TEString &paramsJson)
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
            return "\"Error: Entity " + TEString::FromInt((int)id) + " does not exist\"";

        Entity e((EntityID)id, &manager);
        scene->DestroyEntity(e);
        return "\"Destroyed entity with ID: " + TEString::FromInt((int)id) + "\"";
    }
    return "\"Error: No active scene\"";
}

TEString MCPPlugin::Tool_CreateSprite(const TEString &paramsJson)
{
    TEString name = ExtractString(paramsJson, "name");
    TEString relPath = ExtractString(paramsJson, "path");

    if (!Project::GetActive())
        return "\"Error: No active project\"";
    if (name.empty() || relPath.empty())
        return "\"Error: 'name' and 'path' are required\"";

    TEString fullPath = Project::GetAssetDirectory() / relPath / (name + ".tesprite");
    TEFileSystem::CreateDirectories(fullPath.GetParentPath());

    std::ofstream file(fullPath.c_str());
    if (!file.is_open())
        return "\"Error: Cannot write to " + fullPath + "\"";

    file << "Sprite: " << name.c_str() << "\n";
    file << "Texture: 0\n";
    file << "UVs: 0 0 1 1\n";
    file.close();

    return "\"Created sprite at: " + fullPath + "\"";
}

TEString MCPPlugin::Tool_CreateDirectory(const TEString &paramsJson)
{
    TEString relPath = ExtractString(paramsJson, "path");

    if (!Project::GetActive())
        return "\"Error: No active project\"";
    if (relPath.empty())
        return "\"Error: 'path' is required\"";

    TEString fullPath = Project::GetAssetDirectory() / relPath;
    if (TEFileSystem::CreateDirectories(fullPath) || TEFileSystem::Exists(fullPath))
        return "\"Directory ready: " + fullPath + "\"";

    return "\"Error: Failed to create directory\"";
}

TEString MCPPlugin::Tool_DeletePath(const TEString &paramsJson)
{
    TEString relPath = ExtractString(paramsJson, "path");

    if (!Project::GetActive())
        return "\"Error: No active project\"";
    if (relPath.empty())
        return "\"Error: 'path' is required\"";

    TEString fullPath = Project::GetAssetDirectory() / relPath;
    if (!TEFileSystem::Exists(fullPath))
        return "\"Error: Path not found: " + fullPath + "\"";

    uint64_t count = TEFileSystem::RemoveAll(fullPath);
    return "\"Deleted " + TEString::FromInt((int)count) + " item(s) at: " + fullPath + "\"";
}

// ---------------------------------------------------------------------------
// NEW MCP Tool Implementations
// ---------------------------------------------------------------------------

TEString MCPPlugin::Tool_GetEditorModes()
{
    TEString json = "{\"modes\":[";
    const auto &modes = EditorModeRegistry::GetModes();
    for (size_t i = 0; i < modes.Size(); ++i)
    {
        if (i > 0)
            json += ",";
        json += "{\"name\":\"" + modes[i]->GetName() + "\"}";
    }
    json += "]}";
    return json;
}

TEString MCPPlugin::Tool_SetEditorMode(const TEString &paramsJson)
{
    TEString modeName = "";
    int nameKey = paramsJson.Find("\"name\"");
    if (nameKey != -1)
    {
        int valStart = paramsJson.Find(":", ESearchCase::IgnoreCase, ESearchDir::FromStart, nameKey);
        if (valStart != -1)
        {
            int q1 = paramsJson.Find("\"", ESearchCase::IgnoreCase, ESearchDir::FromStart, valStart);
            if (q1 != -1)
            {
                int q2 = paramsJson.Find("\"", ESearchCase::IgnoreCase, ESearchDir::FromStart, q1 + 1);
                if (q2 != -1)
                {
                    modeName = paramsJson.Substr(q1 + 1, q2 - q1 - 1);
                }
            }
        }
    }

    if (modeName.empty())
        return "\"Error: 'name' property is required\"";

    EditorModeRegistry::SetActiveMode(modeName);
    return "\"Success: Mode switched to '" + modeName + "'\"";
}

TEString MCPPlugin::Tool_GetViewportScreenshot()
{
    // Find EditorLayer
    const auto &layers = Application::Get().GetLayerStack();
    for (Layer *layer : layers)
    {
        EditorLayer *editorLayer = dynamic_cast<EditorLayer *>(layer);
        if (!editorLayer)
            continue;

        // We know from EditorLayer.hpp that m_Framebuffer is of type TERef<Framebuffer>
        // and we have its pointer or we can read from standard RenderCommand screen size.
        // We will read pixels from active bound framebuffer target.
        uint32_t width = Application::Get().GetWindow().GetWidth();
        uint32_t height = Application::Get().GetWindow().GetHeight();

        TEArray<uint32_t> pixels;
        pixels.Resize(width * height, 0);
        RenderCommand::ReadPixelsRGBA(0, 0, width, height, pixels.GetData());

        // Flip image Y coordinate since OpenGL reads from bottom-left
        TEArray<uint32_t> flipped;
        flipped.Resize(width * height, 0);
        for (uint32_t y = 0; y < height; y++)
        {
            memcpy(&flipped[y * width], &pixels[(height - 1 - y) * width], width * 4);
        }

        // Export screenshot file
        TEString screenshotPath = TEFileSystem::GetCurrentWorkingDirectory() / "temp_viewport_capture.png";

        // Save the PNG using built-in engine asset utilities
        AssetManager::ExportImagePNG(screenshotPath.c_str(), width, height, 4, flipped.GetData());

        return "\"Screenshot taken and saved temporarily to: " + screenshotPath + "\"";
    }

    return "\"Error: EditorLayer viewport not found\"";
}

TEString MCPPlugin::Tool_DeleteScreenshot()
{
    TEString screenshotPath = TEFileSystem::GetCurrentWorkingDirectory() / "temp_viewport_capture.png";
    if (TEFileSystem::Exists(screenshotPath))
    {
        TEFileSystem::Remove(screenshotPath);
        return "\"Screenshot deleted successfully\"";
    }
    return "\"Error: No active screenshot found to delete\"";
}

TEString MCPPlugin::Tool_SendEditorInput(const TEString &paramsJson)
{
    TEString type = ExtractString(paramsJson, "type");
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
        return "\"Simulated key_press: " + TEString::FromInt((int)code) + "\"";
    }
    else if (type == "key_release")
    {
        KeyReleasedEvent event((KeyCode)code);
        const auto &layers = Application::Get().GetLayerStack();
        for (auto it = layers.end(); it != layers.begin();)
        {
            (*--it)->OnEvent(event);
        }
        return "\"Simulated key_release: " + TEString::FromInt((int)code) + "\"";
    }
    else if (type == "mouse_press")
    {
        MouseButtonPressedEvent event((MouseCode)code);
        const auto &layers = Application::Get().GetLayerStack();
        for (auto it = layers.end(); it != layers.begin();)
        {
            (*--it)->OnEvent(event);
        }
        return "\"Simulated mouse_press: " + TEString::FromInt((int)code) + "\"";
    }
    else if (type == "mouse_release")
    {
        MouseButtonReleasedEvent event((MouseCode)code);
        const auto &layers = Application::Get().GetLayerStack();
        for (auto it = layers.end(); it != layers.begin();)
        {
            (*--it)->OnEvent(event);
        }
        return "\"Simulated mouse_release: " + TEString::FromInt((int)code) + "\"";
    }

    return "\"Error: Invalid input type. Valid: key_press, key_release, mouse_press, mouse_release\"";
}

TEString MCPPlugin::Tool_SelectEntity(const TEString &paramsJson)
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
            return "\"Error: Entity ID " + TEString::FromInt((int)entityId) + " is not valid in active scene\"";

        Entity entity((EntityID)entityId, &manager);

        // Use reflection method or force selection update inside EditorLayer
        // We know from EditorLayer.hpp: SelectEntity(Entity entity, bool multiSelect = false, bool toggle = false)
        // Let's call it via EditorLayer pointer! (It's marked TE_API / public in EditorLayer.hpp)
        editorLayer->ClearSelection();
        editorLayer->SelectEntity(entity, false, false);
        return "\"Selected entity with ID: " + TEString::FromInt((int)entityId) + "\"";
    }

    return "\"Error: EditorLayer or active scene not found\"";
}

TEString MCPPlugin::Tool_SetEntityProperties(const TEString &paramsJson)
{
    int64_t entityId = ExtractInt(paramsJson, "id");
    TEString propObj = ExtractObject(paramsJson, "properties");

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
            return "\"Error: Entity ID " + TEString::FromInt((int)entityId) + " not found\"";

        // Clean escaped quotes \" to normal quotes " in the entire properties object
        propObj = propObj.Replace("\\\"", "\"");

        int successfulUpdates = 0;
        size_t searchPos = 0;
        while (searchPos < propObj.Len())
        {
            size_t keyStart = TEString::npos;
            for (size_t i = searchPos; i < propObj.Len(); ++i)
            {
                if (propObj[i] == '\"')
                {
                    keyStart = i;
                    break;
                }
            }
            if (keyStart == TEString::npos)
                break;

            size_t keyEnd = TEString::npos;
            for (size_t i = keyStart + 1; i < propObj.Len(); ++i)
            {
                if (propObj[i] == '\"')
                {
                    keyEnd = i;
                    break;
                }
            }
            if (keyEnd == TEString::npos)
                break;

            TEString compName = propObj.Substr(keyStart + 1, keyEnd - keyStart - 1);

            size_t compValStart = TEString::npos;
            for (size_t i = keyEnd + 1; i < propObj.Len(); ++i)
            {
                if (propObj[i] == '{')
                {
                    compValStart = i;
                    break;
                }
            }
            if (compValStart == TEString::npos)
                break;

            size_t compValEnd = compValStart;
            int braceCount = 1;
            while (braceCount > 0 && compValEnd < propObj.Len() - 1)
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

            TEString compPropsJson = propObj.Substr(compValStart, compValEnd - compValStart);
            searchPos = compValEnd; // Update lookahead pointer

            TE_CORE_INFO("[MCPPlugin Diagnostic] compName: {0}, compPropsJson: {1}", compName, compPropsJson);

            // Update matching component instance on the target entity
            TEArray<TComponent *> allComponents = manager.GetAllComponents((EntityID)entityId);
            TE_CORE_INFO("[MCPPlugin Diagnostic] Entity ID: {0}, Total Components found: {1}", entityId,
                         allComponents.Num());
            TComponent *targetComponent = nullptr;
            for (auto *c : allComponents)
            {
                if (c)
                {
                    const auto *meta = Scene::GetGlobalComponentRegistry().GetMetadata(std::type_index(typeid(*c)));
                    TEString cClassName = meta ? meta->ClassName : "";
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
                    TEString posStr = ExtractString(compPropsJson, "Position");
                    TEString rotStr = ExtractString(compPropsJson, "Rotation");
                    TEString scaleStr = ExtractString(compPropsJson, "Scale");

                    TE_CORE_INFO("[MCPPlugin Diagnostic] TransformComponent: posStr={0}, rotStr={1}, scaleStr={2}",
                                 posStr, rotStr, scaleStr);

                    if (!posStr.empty())
                    {
                        auto pParts = posStr.Split(' ');
                        if (pParts.Num() >= 3)
                        {
                            targetComponent->Transform.Position.x = pParts[0].ToFloat();
                            targetComponent->Transform.Position.y = pParts[1].ToFloat();
                            targetComponent->Transform.Position.z = pParts[2].ToFloat();
                            successfulUpdates++;
                        }
                    }
                    if (!rotStr.empty())
                    {
                        auto rParts = rotStr.Split(' ');
                        if (rParts.Num() >= 3)
                        {
                            targetComponent->Transform.Rotation.Pitch = rParts[0].ToFloat();
                            targetComponent->Transform.Rotation.Yaw = rParts[1].ToFloat();
                            targetComponent->Transform.Rotation.Roll = rParts[2].ToFloat();
                            successfulUpdates++;
                        }
                    }
                    if (!scaleStr.empty())
                    {
                        auto sParts = scaleStr.Split(' ');
                        if (sParts.Num() >= 3)
                        {
                            targetComponent->Transform.Scale.Scale.x = sParts[0].ToFloat();
                            targetComponent->Transform.Scale.Scale.y = sParts[1].ToFloat();
                            targetComponent->Transform.Scale.Scale.z = sParts[2].ToFloat();
                            successfulUpdates++;
                        }
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
                            TEString propVal = ExtractString(compPropsJson, propMeta.Name);
                            if (propVal.IsEmpty())
                            {
                                size_t pPos = compPropsJson.Find("\"" + propMeta.Name + "\"");
                                if (pPos != TEString::npos)
                                {
                                    size_t colon = compPropsJson.Find(":", ESearchCase::CaseSensitive,
                                                                      ESearchDir::FromStart, (int)pPos);
                                    if (colon != TEString::npos)
                                    {
                                        size_t valStart = compPropsJson.find_first_not_of(" \t\r\n", colon + 1);
                                        size_t valEnd = TEString::npos;
                                        if (valStart != TEString::npos)
                                        {
                                            for (size_t i = valStart; i < compPropsJson.Len(); ++i)
                                            {
                                                char ch = compPropsJson[i];
                                                if (ch == ',' || ch == '}' || ch == '\r' || ch == '\n')
                                                {
                                                    valEnd = i;
                                                    break;
                                                }
                                            }
                                            propVal = compPropsJson.Substr(valStart, valEnd != TEString::npos
                                                                                         ? valEnd - valStart
                                                                                         : TEString::npos);
                                        }
                                    }
                                }
                            }

                            TE_CORE_INFO("[MCPPlugin Diagnostic] Checking property: {0}, extracted value: {1}",
                                         propMeta.Name, propVal);

                            if (!propVal.IsEmpty() && propMeta.DeserializeFunc)
                            {
                                propMeta.DeserializeFunc(targetComponent, propVal);
                                successfulUpdates++;
                            }
                        }
                    }
                }
            }
        }

        return "\"Updated " + TEString::FromInt(successfulUpdates) + " entity component properties successfully\"";
    }

    return "\"Error: Active EditorLayer or scene context missing\"";
}

TEString MCPPlugin::Tool_AddComponent(const TEString &paramsJson)
{
    int64_t entityId = ExtractInt(paramsJson, "id");
    TEString compType = ExtractString(paramsJson, "type");

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
            return "\"Error: Entity ID " + TEString::FromInt((int)entityId) + " not found\"";

        const auto &factories = manager.GetRegisteredComponents();
        auto it = factories.find(compType);
        if (it == factories.end())
            return "\"Error: Component type '" + compType + "' not registered in EntityManager factories\"";

        // Call the factory to instantiate the component onto the entity
        TComponent *comp = it->second((EntityID)entityId);
        if (comp)
        {
            comp->OnInitialize();
            return "\"Added component '" + compType + "' to entity " + TEString::FromInt((int)entityId) + "\"";
        }
        return "\"Error: Component factory failed to instantiate component\"";
    }

    return "\"Error: Active EditorLayer or scene context missing\"";
}

// ---------------------------------------------------------------------------
// HTTP / SSE helpers
// ---------------------------------------------------------------------------

TEString MCPPlugin::ReadHttpRequest(uintptr_t socket)
{
    TEString result;
    result.reserve(4096);

#ifdef TE_PLATFORM_WINDOWS
    DWORD timeout = 3000;
    setsockopt((SOCKET)socket, SOL_SOCKET, SO_RCVTIMEO, (const char *)&timeout, sizeof(timeout));

    TEString chunk;
    chunk.Reserve(4096);
    int bytes;

    // Phase 1: read until we have complete headers (\r\n\r\n)
    while ((bytes = recv((SOCKET)socket, chunk.Data(), 4095, 0)) > 0)
    {
        chunk.Data()[bytes] = '\0';
        result += chunk.c_str();
        if (result.find("\r\n\r\n") != TEString::npos)
            break;
    }

    if (result.find("\r\n\r\n") == TEString::npos)
        return result; // headers never completed

    TE_CORE_INFO("[MCPPlugin Diagnostic] Raw Headers received:\n{0}", result);

    // Handle "Expect: 100-continue" — client waits for this before sending body
    bool hasExpect = (result.find("Expect: 100-continue") != TEString::npos ||
                      result.find("expect: 100-continue") != TEString::npos);
    if (hasExpect)
    {
        const char *cont = "HTTP/1.1 100 Continue\r\n\r\n";
        send((SOCKET)socket, cont, (int)strlen(cont), 0);
    }

    // Phase 2: read body according to Content-Length
    size_t clPos = result.Find("Content-Length:");
    if (clPos == TEString::npos)
        return result; // no body (GET / OPTIONS)

    size_t clEnd = result.Find("\r\n", ESearchCase::CaseSensitive, ESearchDir::FromStart, (int)clPos);
    int contentLength = 0;
    try
    {
        TEString clVal = result.Substr(clPos + 15, clEnd != TEString::npos ? clEnd - (clPos + 15) : TEString::npos);
        // Remove whitespace
        clVal.TrimInline();
        contentLength = clVal.ToInt();
    }
    catch (...)
    {
        return result;
    }

    size_t headerEnd = result.Find("\r\n\r\n") + 4;
    int bodyReceived = (int)(result.Len() - headerEnd);

    TEArray<char> buffer;
    buffer.Resize(1024, '\0');

    while (bodyReceived < contentLength)
    {
        bytes = recv((SOCKET)socket, buffer.GetData(), (int)buffer.Num() - 1, 0);
        if (bytes <= 0)
            break;
        buffer[bytes] = '\0';
        result += buffer.GetData();
        bodyReceived += bytes;
    }
#endif
    return result;
}

void MCPPlugin::SendHttpResponse(uintptr_t socket, int statusCode, const TEString &contentType, const TEString &body,
                                 bool keepAlive)
{
    TEString statusText = "OK";
    if (statusCode == 202)
        statusText = "Accepted";
    if (statusCode == 204)
        statusText = "No Content";
    if (statusCode == 400)
        statusText = "Bad Request";
    if (statusCode == 404)
        statusText = "Not Found";

    TEString response = "HTTP/1.1 " + TEString::FromInt(statusCode) + " " + statusText +
                        "\r\n"
                        "Content-Type: " +
                        contentType +
                        "\r\n"
                        "Content-Length: " +
                        TEString::FromInt((int)body.size()) +
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

void MCPPlugin::SendSSEEvent(uintptr_t socket, const TEString &eventName, const TEString &data)
{
    // SSE format: "event: <name>\ndata: <data>\n\n"
    TEString sseMsg = "event: " + eventName + "\ndata: " + data + "\n\n";
#ifdef TE_PLATFORM_WINDOWS
    send((SOCKET)socket, sseMsg.c_str(), (int)sseMsg.size(), 0);
#endif
}

void MCPPlugin::BroadcastSSE(const TEString &eventName, const TEString &data)
{
    std::lock_guard<std::mutex> lock(m_SSEMutex);
    for (auto &client : m_SSEClients)
    {
        if (!client.Active)
            continue;

        TEString sseMsg = "event: " + eventName + "\ndata: " + data + "\n\n";
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
    for (int32_t i = (int32_t)m_SSEClients.Num() - 1; i >= 0; --i)
    {
        if (!m_SSEClients[i].Active)
        {
            m_SSEClients.RemoveAt(i);
        }
    }
}

// ---------------------------------------------------------------------------
// JSON helpers
// ---------------------------------------------------------------------------

TEString MCPPlugin::ExtractString(const TEString &json, const TEString &key)
{
    TEString search = "\"" + key + "\"";
    size_t keyPos = json.find(search);
    if (keyPos == TEString::npos)
        return "";

    size_t colon = json.find(':', keyPos + search.size());
    if (colon == TEString::npos)
        return "";

    size_t start = json.find('"', colon + 1);
    if (start == TEString::npos)
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

int64_t MCPPlugin::ExtractInt(const TEString &json, const TEString &key)
{
    TEString search = "\"" + key + "\"";
    size_t keyPos = json.find(search);
    if (keyPos == TEString::npos)
        return -1;

    size_t colon = json.find(':', keyPos + search.size());
    if (colon == TEString::npos)
        return -1;

    size_t valStart = TEString::npos;
    for (size_t i = colon + 1; i < json.Len(); ++i)
    {
        char ch = json[i];
        if (ch == '-' || (ch >= '0' && ch <= '9'))
        {
            valStart = i;
            break;
        }
    }
    if (valStart == TEString::npos)
        return -1;

    size_t valEnd = json.Len();
    for (size_t i = valStart + 1; i < json.Len(); ++i)
    {
        char ch = json[i];
        if (ch < '0' || ch > '9')
        {
            valEnd = i;
            break;
        }
    }
    TEString numStr = json.Substr(valStart, valEnd - valStart);
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
TEString MCPPlugin::ExtractObject(const TEString &json, const TEString &key)
{
    TEString search = "\"" + key + "\"";
    size_t keyPos = json.find(search);
    if (keyPos == TEString::npos)
        return "";

    size_t colon = json.find(':', keyPos + search.size());
    if (colon == TEString::npos)
        return "";

    size_t start = json.find('{', colon + 1);
    if (start == TEString::npos)
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

TEString MCPPlugin::Tool_OpenAssetEditor(const TEString &paramsJson)
{
    TEString path = ExtractString(paramsJson, "path");
    if (path.IsEmpty())
        return "\"Error: 'path' parameter is required\"";

    TEString ext = path.GetExtension();
    if (!ext.IsEmpty() && ext[0] == '.')
        ext = ext.Mid(1);

    auto editor = AssetEditorRegistry::GetEditor(ext);
    if (!editor)
        return "\"Error: No registered asset editor for extension: " + ext + "\"";

    return "\"Opened asset editor for: " + path + "\"";
}
