#pragma once

#include "Core/PreRequisites.h"
#include "Core/Graph/GraphNode.hpp"
#include "Editor/Graph/NodePalettePopup.hpp"
#include "GameFrameWork/GameplayUtils.hpp"
#include "Utils/TEString.hpp"
#include <functional>


class MaterialNodeRegistry
{
public:
    static void RegisterAllNodes(NodePalettePopup &palette);
    static TERef<GraphNode> CreateNodeByType(const TEString &nodeType);
};
