#pragma once

#include "Graph/Graph.hpp"
#include "Core/PCGContext.hpp"
#include "PreRequisites.h"
#include "Graph/PCGNode.hpp"

class PCGGraph : public Graph
{
public:
    PCGGraph(const TEString &name = "PCG Graph");
    virtual ~PCGGraph() override = default;

    bool Execute(PCGExecutionContext &ctx);

    void ResetToDefault();
};
