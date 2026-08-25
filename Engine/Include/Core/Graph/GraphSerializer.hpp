#pragma once

#include "Core/PreRequisites.h"
#include "Core/Graph/Graph.hpp"
#include "Utils/TEString.hpp"
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
