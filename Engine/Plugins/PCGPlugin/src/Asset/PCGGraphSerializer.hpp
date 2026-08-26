#pragma once

#include "Core/PreRequisites.h"
#include "Graph/PCGGraph.hpp"
#include "Utils/TEString.hpp"

class PCGGraphSerializer
{
public:
    static bool SaveToFile(const PCGGraph &graph, const TEString &path);
    static bool LoadFromFile(PCGGraph &graph, const TEString &path);
};
