#include "Core/PreRequisites.h"
#include "Core/Graph/Graph.hpp"
#include "Core/Log.h"
#include <queue>

Graph::Graph(const TEString &name) : m_Name(name) {}

uint64_t Graph::GenerateID() { return m_NextID++; }

TERef<GraphNode> Graph::AddNode(const TERef<GraphNode> &node)
{
    if (!node)
        return nullptr;

    if (node->ID == 0)
        node->ID = GenerateID();

    if (node->ID >= m_NextID)
        m_NextID = node->ID + 1;

    for (auto &pin : node->InputPins)
    {
        if (pin.ID == 0)
            pin.ID = GenerateID();
        if (pin.NodeID == 0)
            pin.NodeID = node->ID;
        if (pin.ID >= m_NextID)
            m_NextID = pin.ID + 1;
    }

    for (auto &pin : node->OutputPins)
    {
        if (pin.ID == 0)
            pin.ID = GenerateID();
        if (pin.NodeID == 0)
            pin.NodeID = node->ID;
        if (pin.ID >= m_NextID)
            m_NextID = pin.ID + 1;
    }

    m_Nodes.Add(node);
    node->OnCreated();
    return node;
}

bool Graph::RemoveNode(uint64_t nodeId)
{
    // Remove all connections associated with this node
    for (size_t i = 0; i < m_Connections.Size();)
    {
        if (m_Connections[i].SourceNodeID == nodeId || m_Connections[i].TargetNodeID == nodeId)
        {
            m_Connections.RemoveAt(i);
        }
        else
        {
            i++;
        }
    }

    for (size_t i = 0; i < m_Nodes.Size(); ++i)
    {
        if (m_Nodes[i]->ID == nodeId)
        {
            m_Nodes.RemoveAt(i);
            return true;
        }
    }
    return false;
}

TERef<GraphNode> Graph::FindNode(uint64_t nodeId) const
{
    for (const auto &node : m_Nodes)
    {
        if (node->ID == nodeId)
            return node;
    }
    return nullptr;
}

GraphPin *Graph::FindPin(uint64_t pinId, TERef<GraphNode> *outNode) const
{
    for (const auto &node : m_Nodes)
    {
        for (auto &pin : const_cast<GraphNode *>(node.get())->InputPins)
        {
            if (pin.ID == pinId)
            {
                if (outNode)
                    *outNode = node;
                return &pin;
            }
        }
        for (auto &pin : const_cast<GraphNode *>(node.get())->OutputPins)
        {
            if (pin.ID == pinId)
            {
                if (outNode)
                    *outNode = node;
                return &pin;
            }
        }
    }
    return nullptr;
}

bool Graph::CanConnect(uint64_t srcPinId, uint64_t dstPinId) const
{
    if (srcPinId == dstPinId)
        return false;

    TERef<GraphNode> srcNode = nullptr;
    TERef<GraphNode> dstNode = nullptr;
    GraphPin *srcPin = FindPin(srcPinId, &srcNode);
    GraphPin *dstPin = FindPin(dstPinId, &dstNode);

    if (!srcPin || !dstPin || !srcNode || !dstNode)
        return false;

    if (!srcPin->CanConnectTo(*dstPin))
        return false;

    // Check if duplicate connection exists
    for (const auto &conn : m_Connections)
    {
        if ((conn.SourcePinID == srcPinId && conn.TargetPinID == dstPinId) ||
            (conn.SourcePinID == dstPinId && conn.TargetPinID == srcPinId))
        {
            return false;
        }
    }

    return true;
}

bool Graph::AddConnection(uint64_t srcPinId, uint64_t dstPinId)
{
    if (!CanConnect(srcPinId, dstPinId))
        return false;

    TERef<GraphNode> srcNode = nullptr;
    TERef<GraphNode> dstNode = nullptr;
    GraphPin *p1 = FindPin(srcPinId, &srcNode);
    GraphPin *p2 = FindPin(dstPinId, &dstNode);

    // Standardize: Source is Output, Target is Input
    uint64_t outNodeId = (p1->Direction == PinDirection::Output) ? srcNode->ID : dstNode->ID;
    uint64_t outPinId = (p1->Direction == PinDirection::Output) ? srcPinId : dstPinId;
    uint64_t inNodeId = (p1->Direction == PinDirection::Input) ? srcNode->ID : dstNode->ID;
    uint64_t inPinId = (p1->Direction == PinDirection::Input) ? srcPinId : dstPinId;

    // Remove any existing connection to single-input pins (if non-flow)
    GraphPin *inPin = (p1->Direction == PinDirection::Input) ? p1 : p2;
    if (inPin->Type.GetKind() != GraphPinKind::Flow)
    {
        for (size_t i = 0; i < m_Connections.Size();)
        {
            if (m_Connections[i].TargetPinID == inPinId)
            {
                m_Connections.RemoveAt(i);
            }
            else
            {
                i++;
            }
        }
    }

    GraphConnection conn(GenerateID(), outNodeId, outPinId, inNodeId, inPinId);
    m_Connections.Add(conn);

    if (srcNode)
        srcNode->OnPinConnected(srcPinId, dstPinId);
    if (dstNode)
        dstNode->OnPinConnected(dstPinId, srcPinId);

    return true;
}

bool Graph::RemoveConnection(uint64_t connectionId)
{
    for (size_t i = 0; i < m_Connections.Size(); ++i)
    {
        if (m_Connections[i].ID == connectionId)
        {
            uint64_t srcPin = m_Connections[i].SourcePinID;
            uint64_t dstPin = m_Connections[i].TargetPinID;
            m_Connections.RemoveAt(i);

            TERef<GraphNode> nodeA, nodeB;
            FindPin(srcPin, &nodeA);
            FindPin(dstPin, &nodeB);
            if (nodeA)
                nodeA->OnPinDisconnected(srcPin);
            if (nodeB)
                nodeB->OnPinDisconnected(dstPin);

            return true;
        }
    }
    return false;
}

bool Graph::DisconnectPin(uint64_t pinId)
{
    bool removed = false;
    for (size_t i = 0; i < m_Connections.Size();)
    {
        if (m_Connections[i].SourcePinID == pinId || m_Connections[i].TargetPinID == pinId)
        {
            m_Connections.RemoveAt(i);
            removed = true;
        }
        else
        {
            i++;
        }
    }
    return removed;
}

TEArray<GraphConnection> Graph::GetConnectionsForPin(uint64_t pinId) const
{
    TEArray<GraphConnection> result;
    for (const auto &conn : m_Connections)
    {
        if (conn.SourcePinID == pinId || conn.TargetPinID == pinId)
            result.Add(conn);
    }
    return result;
}

TEArray<GraphConnection> Graph::GetConnectionsForNode(uint64_t nodeId) const
{
    TEArray<GraphConnection> result;
    for (const auto &conn : m_Connections)
    {
        if (conn.SourceNodeID == nodeId || conn.TargetNodeID == nodeId)
            result.Add(conn);
    }
    return result;
}

bool Graph::HasCycle() const
{
    TEArray<TERef<GraphNode>> order;
    return !GetTopologicalOrder(order);
}

bool Graph::GetTopologicalOrder(TEArray<TERef<GraphNode>> &outOrder) const
{
    outOrder.Empty();
    if (m_Nodes.IsEmpty())
        return true;

    TEMap<uint64_t, int> inDegree;
    TEMap<uint64_t, TEArray<uint64_t>> adj;

    for (const auto &node : m_Nodes)
    {
        inDegree[node->ID] = 0;
        adj[node->ID] = TEArray<uint64_t>();
    }

    for (const auto &conn : m_Connections)
    {
        // Edge from SourceNode to TargetNode
        adj[conn.SourceNodeID].Add(conn.TargetNodeID);
        inDegree[conn.TargetNodeID]++;
    }

    std::queue<uint64_t> q;
    for (const auto &pair : inDegree)
    {
        if (pair.second == 0)
            q.push(pair.first);
    }

    size_t visitedCount = 0;
    while (!q.empty())
    {
        uint64_t u = q.front();
        q.pop();
        visitedCount++;

        auto node = FindNode(u);
        if (node)
            outOrder.Add(node);

        for (uint64_t v : adj[u])
        {
            inDegree[v]--;
            if (inDegree[v] == 0)
                q.push(v);
        }
    }

    return visitedCount == m_Nodes.Size();
}

void Graph::Clear()
{
    m_Connections.Empty();
    m_Nodes.Empty();
    m_NextID = 1;
}
