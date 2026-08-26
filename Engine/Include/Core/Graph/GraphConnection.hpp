#pragma once

#include "Core/PreRequisites.h"
#include <cstdint>

struct TE_API GraphConnection
{
    uint64_t ID = 0;
    uint64_t SourceNodeID = 0;
    uint64_t SourcePinID = 0;
    uint64_t TargetNodeID = 0;
    uint64_t TargetPinID = 0;

    GraphConnection() = default;
    GraphConnection(uint64_t id, uint64_t srcNode, uint64_t srcPin, uint64_t dstNode, uint64_t dstPin)
        : ID(id), SourceNodeID(srcNode), SourcePinID(srcPin), TargetNodeID(dstNode), TargetPinID(dstPin)
    {
    }

    bool operator==(const GraphConnection &other) const
    {
        return SourcePinID == other.SourcePinID && TargetPinID == other.TargetPinID;
    }
};
