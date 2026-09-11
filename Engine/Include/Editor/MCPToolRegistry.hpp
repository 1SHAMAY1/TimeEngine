#pragma once

#include "Core/PreRequisites.h"
#include "GameFrameWork/GameplayUtils.hpp"
#include <functional>
#include <map>
#include <mutex>

struct TE_API MCPToolDefinition
{
    TEString Name;
    TEString Description;
    TEString InputSchemaJson;
    std::function<TEString(const TEString &paramsJson)> Handler;
};

class TE_API MCPToolRegistry
{
public:
    static void RegisterTool(const MCPToolDefinition &tool);
    static void UnregisterTool(const TEString &toolName);
    static void Clear();
    static const TEArray<MCPToolDefinition> &GetTools();
    static bool ExecuteTool(const TEString &toolName, const TEString &paramsJson, TEString &outResult);

private:
    static std::mutex &GetMutex();
    static TEMap<TEString, MCPToolDefinition> &GetToolsMap();
    static TEArray<MCPToolDefinition> &GetToolsList();
};

#define TE_REGISTER_MCP_TOOL(name, description, schemaJson, handlerFn)                                                 \
    namespace                                                                                                          \
    {                                                                                                                  \
    struct MCPToolAutoReg_##name                                                                                       \
    {                                                                                                                  \
        MCPToolAutoReg_##name() { MCPToolRegistry::RegisterTool({#name, description, schemaJson, handlerFn}); }        \
    } s_MCPToolAutoReg_##name;                                                                                         \
    }
