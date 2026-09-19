#pragma once

#include "Graph/DialogueGraph.hpp"
#include "Graph/DialogueGraphNode.hpp"
#include <cstdint>

class DialogueNodeInspector
{
public:
    DialogueNodeInspector() = default;
    ~DialogueNodeInspector() = default;

    void DrawInspector(DialogueGraph &graph, uint64_t selectedNodeId);
};
