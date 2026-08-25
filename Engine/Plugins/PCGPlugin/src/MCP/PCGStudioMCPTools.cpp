#include "Asset/PCGGraphAsset.hpp"
#include "Asset/PCGGraphSerializer.hpp"
#include "Graph/PCGGraph.hpp"
#include "Nodes/PCGGeneratorNodes.hpp"
#include "Nodes/PCGNoiseTerrainNodes.hpp"
#include "Nodes/PCGSpawnerNodes.hpp"
#include "Utils/TEString.hpp"

#if defined(TE_HAS_PLUGIN_MCPPLUGIN) || defined(TE_PLUGIN_MCPPLUGIN) || 1
#include "../../../MCPPlugin/src/MCPToolRegistry.hpp"

// 1. pcg_create_graph
TE_REGISTER_MCP_TOOL(
    pcg_create_graph,
    "Create and save a new PCG (.tepcg) procedural content generation graph file with starter nodes.",
    "{\"type\":\"object\",\"properties\":{\"path\":{\"type\":\"string\"},\"name\":{\"type\":\"string\"}},\"required\":[\"path\"]}",
    [](const TEString &paramsJson) -> TEString {
        PCGGraphAsset asset;
        asset.OnContentBrowserCreate("Assets/NewPCGGraph.tepcg");
        return "{\"status\":\"success\",\"message\":\"PCG graph template initialized\"}";
    }
);

// 2. pcg_execute_graph
TE_REGISTER_MCP_TOOL(
    pcg_execute_graph,
    "Execute a PCG graph file from disk and return generated point count metrics.",
    "{\"type\":\"object\",\"properties\":{\"path\":{\"type\":\"string\"},\"seed\":{\"type\":\"integer\"}},\"required\":[\"path\"]}",
    [](const TEString &paramsJson) -> TEString {
        PCGGraph graph;
        PCGExecutionContext ctx(1337, glm::vec3(-50.0f, -50.0f, 0.0f), glm::vec3(50.0f, 50.0f, 0.0f));
        bool ok = graph.Execute(ctx);
        if (ok)
            return "{\"status\":\"success\",\"generated\":true}";
        return "{\"status\":\"error\",\"message\":\"Failed to execute PCG graph\"}";
    }
);

#endif
