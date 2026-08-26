#pragma once

#include "Core/PreRequisites.h"
#include "Editor/Graph/NodePalettePopup.hpp"

class PCGNodeRegistry
{
public:
    static void RegisterAllNodes(NodePalettePopup &palette);
};
