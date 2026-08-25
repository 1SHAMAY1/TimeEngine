#pragma once

#include "Core/Plugin/IPlugin.hpp"
#include "Core/PreRequisites.h"
#include "GameFrameWork/GameplayUtils.hpp"
#include <atomic>
#include <functional>
#include <mutex>
#include <thread>


// Represents a connected SSE client waiting for server-sent events
struct SSEClient
{
    uintptr_t Socket = 0; // SOCKET handle
    bool Active = true;
};

class MCPPlugin : public IPlugin
{
public:
    virtual void OnLoad() override;
    virtual void OnUnload() override;

    virtual TEString GetName() const override { return "MCPPlugin"; }
    virtual TEString GetVersion() const override { return "1.0.0"; }
    virtual TEString GetAuthor() const override { return "TimeEngine Team"; }
    virtual TEString GetDescription() const override { return "Model Context Protocol Integration & AI Automation Plugin"; }

    virtual void DrawThumbnail(TimeGUIDrawList &dl, const TEVector2 &min, const TEVector2 &max) const override;

private:
    // Main server accept loop
    void ServerThreadMain();

    // Handle a single incoming HTTP connection
    void HandleConnection(uintptr_t clientSocket);

    // HTTP helpers
    TEString ReadHttpRequest(uintptr_t socket);
    void SendHttpResponse(uintptr_t socket, int statusCode, const TEString &contentType, const TEString &body,
                          bool keepAlive = false);
    void SendSSEEvent(uintptr_t socket, const TEString &eventName, const TEString &data);

    // MCP tool dispatch — returns the JSON result string
    TEString DispatchToolCall(const TEString &toolName, const TEString &paramsJson);

    // Individual tool handlers
    TEString Tool_GetEngineInfo();
    TEString Tool_GetSceneHierarchy();
    TEString Tool_CreateEntity(const TEString &paramsJson);
    TEString Tool_DestroyEntity(const TEString &paramsJson);
    TEString Tool_CreateSprite(const TEString &paramsJson);
    TEString Tool_CreateDirectory(const TEString &paramsJson);
    TEString Tool_DeletePath(const TEString &paramsJson);

    // NEW Tool Handlers
    TEString Tool_GetEditorModes();
    TEString Tool_SetEditorMode(const TEString &paramsJson);
    TEString Tool_GetViewportScreenshot();
    TEString Tool_SendEditorInput(const TEString &paramsJson);
    TEString Tool_SelectEntity(const TEString &paramsJson);
    TEString Tool_SetEntityProperties(const TEString &paramsJson);
    TEString Tool_AddComponent(const TEString &paramsJson);
    TEString Tool_DeleteScreenshot();
    TEString Tool_OpenAssetEditor(const TEString &paramsJson);

    // JSON parsing helpers
    static TEString ExtractString(const TEString &json, const TEString &key);
    static int64_t ExtractInt(const TEString &json, const TEString &key);
    static TEString ExtractObject(const TEString &json, const TEString &key);

    // SSE broadcast helpers
    void BroadcastSSE(const TEString &eventName, const TEString &data);
    void CleanupSSEClients();

private:
    std::thread m_ServerThread;
    std::atomic<bool> m_Running{false};
    uintptr_t m_ListenSocket = 0; // SOCKET

    std::mutex m_SSEMutex;
    TEArray<SSEClient> m_SSEClients;
};

