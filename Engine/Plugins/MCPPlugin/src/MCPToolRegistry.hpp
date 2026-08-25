#pragma once

#include "Core/PreRequisites.h"
#include "GameFrameWork/GameplayUtils.hpp"
#include <functional>
#include <map>
#include <mutex>


struct MCPToolDefinition
{
    TEString Name;
    TEString Description;
    TEString InputSchemaJson;
    std::function<TEString(const TEString &paramsJson)> Handler;
};

class MCPToolRegistry
{
public:
    static void RegisterTool(const MCPToolDefinition &tool)
    {
        std::lock_guard<std::mutex> lock(GetMutex());
        auto &map = GetToolsMap();
        if (map.find(tool.Name) == map.end())
        {
            GetToolsList().push_back(tool);
            map[tool.Name] = tool;
        }
        else
        {
            map[tool.Name] = tool;
            for (auto &t : GetToolsList())
            {
                if (t.Name == tool.Name)
                {
                    t = tool;
                    break;
                }
            }
        }
    }

    static void UnregisterTool(const TEString &toolName)
    {
        std::lock_guard<std::mutex> lock(GetMutex());
        GetToolsMap().erase(toolName);
        auto &list = GetToolsList();
        for (auto it = list.begin(); it != list.end(); ++it)
        {
            if (it->Name == toolName)
            {
                list.erase(it);
                break;
            }
        }
    }

    static const TEArray<MCPToolDefinition> &GetTools()
    {
        std::lock_guard<std::mutex> lock(GetMutex());
        return GetToolsList();
    }

    static bool ExecuteTool(const TEString &toolName, const TEString &paramsJson, TEString &outResult)
    {
        std::lock_guard<std::mutex> lock(GetMutex());
        auto &map = GetToolsMap();
        auto it = map.find(toolName);
        if (it != map.end() && it->second.Handler)
        {
            outResult = it->second.Handler(paramsJson);
            return true;
        }
        return false;
    }

private:
    static std::mutex &GetMutex()
    {
        static std::mutex s_Mutex;
        return s_Mutex;
    }

    static TEMap<TEString, MCPToolDefinition> &GetToolsMap()
    {
        static TEMap<TEString, MCPToolDefinition> s_Map;
        return s_Map;
    }

    static TEArray<MCPToolDefinition> &GetToolsList()
    {
        static TEArray<MCPToolDefinition> s_List;
        return s_List;
    }
};

#define TE_REGISTER_MCP_TOOL(name, description, schemaJson, handlerFn)                                                 \
    namespace                                                                                                          \
    {                                                                                                                  \
    struct MCPToolAutoReg_##name                                                                                       \
    {                                                                                                                  \
        MCPToolAutoReg_##name()                                                                                        \
        {                                                                                                              \
            MCPToolRegistry::RegisterTool({#name, description, schemaJson, handlerFn});                          \
        }                                                                                                              \
    } s_MCPToolAutoReg_##name;                                                                                         \
    }

