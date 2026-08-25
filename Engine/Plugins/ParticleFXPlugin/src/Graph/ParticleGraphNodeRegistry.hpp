#pragma once

#include "Core/PreRequisites.h"
#include "Core/Graph/GraphNode.hpp"
#include "Editor/Graph/NodePalettePopup.hpp"


class ParticleGraphNodeRegistry
{
public:
    static void RegisterAllNodes(NodePalettePopup &palette);
    static TERef<GraphNode> CreateNodeByType(const TEString &nodeType);
};
