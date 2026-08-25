#include "SoundGraph.hpp"
#include <algorithm>
#include <set>

namespace SoundStudio {

SoundGraph::SoundGraph()
{
}

SoundGraph::~SoundGraph()
{
    Clear();
}

void SoundGraph::Clear()
{
    m_Nodes.clear();
    m_Connections.clear();
    m_Parameters.clear();
    m_NextNodeId = 1;
    m_NextConnectionId = 1;
}

uint64_t SoundGraph::AddNode(TERef<SoundGraphNode> node)
{
    if (!node)
        return 0;

    uint64_t id = m_NextNodeId++;
    node->Initialize(id);
    m_Nodes[id] = node;
    return id;
}

void SoundGraph::RemoveNode(uint64_t nodeId)
{
    auto it = m_Nodes.find(nodeId);
    if (it != m_Nodes.end())
    {
        // Disconnect all pins belonging to this node
        for (const auto &p : it->second->GetInputs())
            DisconnectPin(p.ID);
        for (const auto &p : it->second->GetOutputs())
            DisconnectPin(p.ID);

        m_Nodes.erase(it);
    }
}

TERef<SoundGraphNode> SoundGraph::GetNode(uint64_t nodeId)
{
    auto it = m_Nodes.find(nodeId);
    return (it != m_Nodes.end()) ? it->second : nullptr;
}

uint64_t SoundGraph::ConnectPins(uint64_t outputPinId, uint64_t inputPinId)
{
    // Disconnect any existing connection to inputPinId (single source per input pin)
    DisconnectPin(inputPinId);

    SoundConnection conn;
    conn.ID = m_NextConnectionId++;
    conn.OutputPinID = outputPinId;
    conn.InputPinID = inputPinId;
    m_Connections.push_back(conn);

    return conn.ID;
}

void SoundGraph::DisconnectPins(uint64_t connectionId)
{
    m_Connections.erase(
        std::remove_if(m_Connections.begin(), m_Connections.end(),
                       [connectionId](const SoundConnection &c) { return c.ID == connectionId; }),
        m_Connections.end());
}

void SoundGraph::DisconnectPin(uint64_t pinId)
{
    m_Connections.erase(
        std::remove_if(m_Connections.begin(), m_Connections.end(),
                       [pinId](const SoundConnection &c) { return c.InputPinID == pinId || c.OutputPinID == pinId; }),
        m_Connections.end());
}

SoundPin *SoundGraph::FindPin(uint64_t pinId)
{
    for (auto &pair : m_Nodes)
    {
        if (auto *pin = pair.second->FindPin(pinId))
            return pin;
    }
    return nullptr;
}

TEArray<TERef<SoundGraphNode>> SoundGraph::GetExecutionOrder()
{
    TEArray<TERef<SoundGraphNode>> order;
    TESet<uint64_t> visited;

    // Simple dependency walk: Nodes without inputs or outputs first, then DAG
    for (auto &pair : m_Nodes)
    {
        order.push_back(pair.second);
    }

    return order;
}

void SoundGraph::SetParameterFloat(const TEString &paramName, float value)
{
    m_Parameters[paramName] = value;
}

float SoundGraph::GetParameterFloat(const TEString &paramName) const
{
    auto it = m_Parameters.find(paramName);
    return (it != m_Parameters.end()) ? it->second : 0.0f;
}

void SoundGraph::ResetAllNodes()
{
    for (auto &pair : m_Nodes)
    {
        if (pair.second)
            pair.second->ResetState();
    }
}

} // namespace SoundStudio
