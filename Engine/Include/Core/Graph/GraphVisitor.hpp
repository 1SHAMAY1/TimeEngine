#pragma once

#include "Core/PreRequisites.h"
#include "Core/Graph/GraphNode.hpp"


class TE_API GraphVisitor
{
public:
    virtual ~GraphVisitor() = default;
    virtual void VisitNode(const TERef<GraphNode> &node) = 0;
    virtual void BeginGraph(const class Graph &graph) {}
    virtual void EndGraph(const class Graph &graph) {}
};
