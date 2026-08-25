#include "Graph/PCGGraph.hpp"
#include "Core/Log.h"

PCGGraph::PCGGraph(const TEString &name)
    : Graph(name)
{
}

void PCGGraph::ResetToDefault()
{
    Clear();
}

bool PCGGraph::Execute(PCGExecutionContext &ctx)
{
    ctx.ClearCache();

    TEArray<TERef<GraphNode>> executionOrder;
    if (!GetTopologicalOrder(executionOrder))
    {
        TE_CORE_ERROR("[PCGGraph] Failed to execute graph: Cycle detected!");
        return false;
    }

    for (size_t i = 0; i < executionOrder.Size(); ++i)
    {
        if (ctx.Cancelled.load())
        {
            TE_CORE_WARN("[PCGGraph] Execution cancelled by user or engine.");
            return false;
        }

        auto node = executionOrder[i];
        if (!node)
            continue;

        // Propagate data from incoming connections into this node's input pins
        for (size_t p = 0; p < node->InputPins.Size(); ++p)
        {
            uint64_t inPinId = node->InputPins[p].ID;
            for (size_t c = 0; c < m_Connections.Size(); ++c)
            {
                if (m_Connections[c].TargetPinID == inPinId)
                {
                    uint64_t srcPinId = m_Connections[c].SourcePinID;
                    PCGPointDataRef srcData = ctx.GetPinData(srcPinId);
                    if (srcData)
                    {
                        ctx.SetPinData(inPinId, srcData);
                    }
                    break;
                }
            }
        }

        auto pcgNode = std::dynamic_pointer_cast<PCGNode>(node);
        if (pcgNode)
        {
            if (!pcgNode->Execute(ctx))
            {
                TE_CORE_WARN("[PCGGraph] Node '{}' (ID: {}) failed execution.", pcgNode->Title, pcgNode->ID);
            }
        }
    }

    return true;
}
