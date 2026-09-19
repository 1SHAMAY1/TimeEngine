#pragma once

#include "Graph/GraphNode.hpp"
#include "PreRequisites.h"
#include "Editor/Graph/NodePalettePopup.hpp"

class ParticleGraphNodeRegistry
{
public:
    static void RegisterAllNodes(NodePalettePopup &palette);
    static TERef<GraphNode> CreateNodeByType(const TEString &nodeType);
};
