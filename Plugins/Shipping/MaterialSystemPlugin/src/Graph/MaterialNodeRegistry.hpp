#pragma once

#include "Graph/GraphNode.hpp"
#include "PreRequisites.h"
#include "Editor/Graph/NodePalettePopup.hpp"
#include "GameplayUtils.hpp"
#include "EngineTypes/TEString.hpp"
#include <functional>

class MaterialNodeRegistry
{
public:
    static void RegisterAllNodes(NodePalettePopup &palette);
    static TERef<GraphNode> CreateNodeByType(const TEString &nodeType);
};
