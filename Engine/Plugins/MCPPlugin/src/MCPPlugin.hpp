#pragma once

#include "Core/Plugin/IPlugin.hpp"
#include <thread>
#include <atomic>
#include <mutex>
#include <vector>
#include <string>
#include <functional>

namespace TE
{

// Represents a connected SSE client waiting for server-sent events
struct SSEClient
{
    uintptr_t Socket = 0; // SOCKET handle
    bool      Active = true;
};

class MCPPlugin : public IPlugin
{
public:
    virtual void OnLoad() override;
    virtual void OnUnload() override;

private:
    // Main server accept loop
    void ServerThreadMain();

    // Handle a single incoming HTTP connection
    void HandleConnection(uintptr_t clientSocket);

    // HTTP helpers
    std::string ReadHttpRequest(uintptr_t socket);
    void        SendHttpResponse(uintptr_t socket, int statusCode, const std::string& contentType,
                                 const std::string& body, bool keepAlive = false);
    void        SendSSEEvent(uintptr_t socket, const std::string& eventName, const std::string& data);

    // MCP tool dispatch — returns the JSON result string
    std::string DispatchToolCall(const std::string& toolName, const std::string& paramsJson);

    // Individual tool handlers
    std::string Tool_GetEngineInfo();
    std::string Tool_GetSceneHierarchy();
    std::string Tool_CreateEntity(const std::string& paramsJson);
    std::string Tool_DestroyEntity(const std::string& paramsJson);
    std::string Tool_CreateSprite(const std::string& paramsJson);
    std::string Tool_CreateDirectory(const std::string& paramsJson);
    std::string Tool_DeletePath(const std::string& paramsJson);

    // NEW Tool Handlers
    std::string Tool_GetEditorModes();
    std::string Tool_SetEditorMode(const std::string& paramsJson);
    std::string Tool_GetViewportScreenshot();
    std::string Tool_SendEditorInput(const std::string& paramsJson);
    std::string Tool_SelectEntity(const std::string& paramsJson);
    std::string Tool_SetEntityProperties(const std::string& paramsJson);
    std::string Tool_AddComponent(const std::string& paramsJson);
    std::string Tool_DeleteScreenshot();

    // JSON parsing helpers
    static std::string ExtractString(const std::string& json, const std::string& key);
    static int64_t     ExtractInt(const std::string& json, const std::string& key);
    static std::string ExtractObject(const std::string& json, const std::string& key);

    // SSE broadcast helpers
    void BroadcastSSE(const std::string& eventName, const std::string& data);
    void CleanupSSEClients();

private:
    std::thread       m_ServerThread;
    std::atomic<bool> m_Running{ false };
    uintptr_t         m_ListenSocket = 0; // SOCKET

    std::mutex           m_SSEMutex;
    std::vector<SSEClient> m_SSEClients;
};

} // namespace TE
