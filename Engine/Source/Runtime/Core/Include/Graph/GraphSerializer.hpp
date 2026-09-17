#pragma once

#include "Graph/Graph.hpp"
#include "PreRequisites.h"
#include "EngineTypes/TEString.hpp"
#include <functional>

using NodeFactoryFn = std::function<TERef<GraphNode>(const TEString &nodeType)>;

class TE_API GraphSerializer
{
public:
    GraphSerializer(Graph &graph);

    bool Serialize(const TEString &filepath);
    bool Deserialize(const TEString &filepath, NodeFactoryFn factory = nullptr);

    bool SerializeToString(TEString &outText);
    bool DeserializeFromString(const TEString &text, NodeFactoryFn factory = nullptr);

private:
    Graph &m_Graph;
};
