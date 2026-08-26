#pragma once

#include "Core/Graph/GraphConnection.hpp"
#include "Core/Graph/GraphNode.hpp"
#include "Core/Graph/GraphPin.hpp"
#include "Core/PreRequisites.h"
#include "GameFrameWork/GameplayUtils.hpp"
#include "Utils/TEString.hpp"

class TE_API Graph
{
public:
    Graph(const TEString &name = "Untitled Graph");
    virtual ~Graph() = default;

    uint64_t GenerateID();

    const TEString &GetName() const { return m_Name; }
    void SetName(const TEString &name) { m_Name = name; }

    // Node Management
    TERef<GraphNode> AddNode(const TERef<GraphNode> &node);
    bool RemoveNode(uint64_t nodeId);
    TERef<GraphNode> FindNode(uint64_t nodeId) const;
    const TEArray<TERef<GraphNode>> &GetNodes() const { return m_Nodes; }
    TEArray<TERef<GraphNode>> &GetNodes() { return m_Nodes; }

    // Pin Lookup
    GraphPin *FindPin(uint64_t pinId, TERef<GraphNode> *outNode = nullptr) const;

    // Connection Management
    bool CanConnect(uint64_t srcPinId, uint64_t dstPinId) const;
    bool AddConnection(uint64_t srcPinId, uint64_t dstPinId);
    bool RemoveConnection(uint64_t connectionId);
    bool DisconnectPin(uint64_t pinId);
    const TEArray<GraphConnection> &GetConnections() const { return m_Connections; }
    TEArray<GraphConnection> &GetConnections() { return m_Connections; }

    TEArray<GraphConnection> GetConnectionsForPin(uint64_t pinId) const;
    TEArray<GraphConnection> GetConnectionsForNode(uint64_t nodeId) const;

    // Topological & DAG Utilities
    bool HasCycle() const;
    bool GetTopologicalOrder(TEArray<TERef<GraphNode>> &outOrder) const;

    // Cleanup
    void Clear();

    uint64_t GetNextID() const { return m_NextID; }
    void SetNextID(uint64_t nextId) { m_NextID = nextId; }

protected:
    TEString m_Name;
    uint64_t m_NextID = 1;
    TEArray<TERef<GraphNode>> m_Nodes;
    TEArray<GraphConnection> m_Connections;
};
