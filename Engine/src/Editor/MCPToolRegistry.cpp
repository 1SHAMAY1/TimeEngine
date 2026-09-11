#include "Core/PreRequisites.h"
#include "Editor/MCPToolRegistry.hpp"

std::mutex &MCPToolRegistry::GetMutex()
{
    static std::mutex s_Mutex;
    return s_Mutex;
}

TEMap<TEString, MCPToolDefinition> &MCPToolRegistry::GetToolsMap()
{
    static TEMap<TEString, MCPToolDefinition> s_Map;
    return s_Map;
}

TEArray<MCPToolDefinition> &MCPToolRegistry::GetToolsList()
{
    static TEArray<MCPToolDefinition> s_List;
    return s_List;
}

void MCPToolRegistry::RegisterTool(const MCPToolDefinition &tool)
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

void MCPToolRegistry::UnregisterTool(const TEString &toolName)
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

void MCPToolRegistry::Clear()
{
    std::lock_guard<std::mutex> lock(GetMutex());
    GetToolsMap().clear();
    GetToolsList().clear();
}

const TEArray<MCPToolDefinition> &MCPToolRegistry::GetTools()
{
    std::lock_guard<std::mutex> lock(GetMutex());
    return GetToolsList();
}

bool MCPToolRegistry::ExecuteTool(const TEString &toolName, const TEString &paramsJson, TEString &outResult)
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
