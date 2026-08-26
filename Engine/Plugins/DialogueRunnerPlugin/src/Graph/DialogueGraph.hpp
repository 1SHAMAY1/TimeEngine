#pragma once

#include "Core/Core.h"
#include "GameFrameWork/GameplayUtils.hpp"
#include "Graph/DialogueGraphConnection.hpp"
#include "Graph/DialogueGraphNode.hpp"
#include "NarrativeTypes.hpp"

class DialogueGraph
{
public:
    DialogueGraph();
    ~DialogueGraph() = default;

    uint64_t GenerateID();

    DialogueGraphNode *AddNode(NarrativeNodeType type, const TEVector2 &position, const TEString &title = "");
    bool RemoveNode(uint64_t nodeId);
    DialogueGraphNode *FindNode(uint64_t nodeId);
    const DialogueGraphNode *FindNode(uint64_t nodeId) const;

    GraphPin *FindPin(uint64_t pinId);
    const GraphPin *FindPin(uint64_t pinId) const;

    uint64_t Connect(uint64_t srcNodeId, uint64_t srcPinId, uint64_t dstNodeId, uint64_t dstPinId);
    bool Disconnect(uint64_t connectionId);
    bool DisconnectPin(uint64_t pinId);

    const TEArray<DialogueGraphNode> &GetNodes() const { return m_Nodes; }
    TEArray<DialogueGraphNode> &GetNodes() { return m_Nodes; }
    const TEArray<DialogueGraphConnection> &GetConnections() const { return m_Connections; }
    TEArray<DialogueGraphConnection> &GetConnections() { return m_Connections; }

    uint64_t GetEntryNodeID() const { return m_EntryNodeID; }
    void SetEntryNodeID(uint64_t id) { m_EntryNodeID = id; }

    uint64_t GetNextNode(uint64_t currentNodeId, uint64_t pinId = 0) const;
    TEArray<DialogueGraphConnection> GetConnectionsFromNode(uint64_t nodeId) const;

    void Clear();
    void ResetToDefaultEntry();

private:
    uint64_t m_NextID = 1;
    uint64_t m_EntryNodeID = 0;
    TEArray<DialogueGraphNode> m_Nodes;
    TEArray<DialogueGraphConnection> m_Connections;
};
