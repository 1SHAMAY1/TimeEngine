#pragma once

#include "Core/Graph/GraphNode.hpp"
#include "Core/PreRequisites.h"
#include "Editor/Graph/NodePalettePopup.hpp"

class ParticleGraphNodeRegistry
{
public:
    static void RegisterAllNodes(NodePalettePopup &palette);
    static TERef<GraphNode> CreateNodeByType(const TEString &nodeType);
};
