#pragma once

#include "Core/Graph/GraphConnection.hpp"


struct DialogueGraphConnection : public GraphConnection
{
    DialogueGraphConnection() = default;
    DialogueGraphConnection(uint64_t id, uint64_t srcNode, uint64_t srcPin, uint64_t dstNode, uint64_t dstPin)
        : GraphConnection(id, srcNode, srcPin, dstNode, dstPin)
    {
    }
};
