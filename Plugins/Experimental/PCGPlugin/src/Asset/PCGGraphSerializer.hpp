#pragma once

#include "PreRequisites.h"
#include "Graph/PCGGraph.hpp"
#include "EngineTypes/TEString.hpp"

class PCGGraphSerializer
{
public:
    static bool SaveToFile(const PCGGraph &graph, const TEString &path);
    static bool LoadFromFile(PCGGraph &graph, const TEString &path);
};
