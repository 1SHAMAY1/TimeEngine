#include "DialogueGraph.hpp"
#include <algorithm>


DialogueGraph::DialogueGraph()
{
    ResetToDefaultEntry();
}

uint64_t DialogueGraph::GenerateID()
{
    return m_NextID++;
}

DialogueGraphNode *DialogueGraph::AddNode(NarrativeNodeType type, const TEVector2 &position,
                                         const TEString &title)
{
    DialogueGraphNode node;
    node.ID = GenerateID();
    node.Type = type;
    node.Position = position;

    switch (type)
    {
    case NarrativeNodeType::Entry:
        node.Title = title.empty() ? "Entry Node" : title;
        node.Size = {180.0f, 90.0f};
        node.OutputPins.Add(DialogueGraphPin(GenerateID(), node.ID, "Out", PinType::Flow, PinDirection::Output));
        if (m_EntryNodeID == 0)
            m_EntryNodeID = node.ID;
        break;

    case NarrativeNodeType::Dialogue:
        node.Title = title.empty() ? "Dialogue Line" : title;
        node.Speaker = "NPC";
        node.Text = "Hello traveler! Welcome to our village.";
        node.Size = {240.0f, 150.0f};
        node.InputPins.Add(DialogueGraphPin(GenerateID(), node.ID, "In", PinType::Flow, PinDirection::Input));
        node.OutputPins.Add(DialogueGraphPin(GenerateID(), node.ID, "Out", PinType::Flow, PinDirection::Output));
        break;

    case NarrativeNodeType::Choice: {
        node.Title = title.empty() ? "Branching Choices" : title;
        node.Speaker = "Player";
        node.Size = {250.0f, 170.0f};
        node.InputPins.Add(DialogueGraphPin(GenerateID(), node.ID, "In", PinType::Flow, PinDirection::Input));

        // Default 2 choices
        StoryChoice c1;
        c1.Index = 0;
        c1.Text = "Tell me more about this place.";
        node.Choices.Add(c1);
        node.OutputPins.Add(
            DialogueGraphPin(GenerateID(), node.ID, "Choice 1", PinType::ChoiceOption, PinDirection::Output));

        StoryChoice c2;
        c2.Index = 1;
        c2.Text = "I must be moving on. Goodbye.";
        node.Choices.Add(c2);
        node.OutputPins.Add(
            DialogueGraphPin(GenerateID(), node.ID, "Choice 2", PinType::ChoiceOption, PinDirection::Output));
        break;
    }

    case NarrativeNodeType::Condition:
        node.Title = title.empty() ? "Branch Check" : title;
        node.ConditionVar = "player_has_key";
        node.ConditionOp = ComparisonOp::Equal;
        node.ConditionValue = NarrativeValue(true);
        node.Size = {220.0f, 130.0f};
        node.InputPins.Add(DialogueGraphPin(GenerateID(), node.ID, "In", PinType::Flow, PinDirection::Input));
        node.OutputPins.Add(DialogueGraphPin(GenerateID(), node.ID, "True", PinType::Flow, PinDirection::Output));
        node.OutputPins.Add(DialogueGraphPin(GenerateID(), node.ID, "False", PinType::Flow, PinDirection::Output));
        break;

    case NarrativeNodeType::Action:
        node.Title = title.empty() ? "Action / Event" : title;
        node.ActionVar = "gold_amount";
        node.ActionOp = MutationOp::Add;
        node.ActionValue = NarrativeValue(50);
        node.Size = {220.0f, 130.0f};
        node.InputPins.Add(DialogueGraphPin(GenerateID(), node.ID, "In", PinType::Flow, PinDirection::Input));
        node.OutputPins.Add(DialogueGraphPin(GenerateID(), node.ID, "Out", PinType::Flow, PinDirection::Output));
        break;

    case NarrativeNodeType::Divert:
        node.Title = title.empty() ? "Divert to Knot" : title;
        node.DivertTargetKnot = "main_quest_part2";
        node.Size = {200.0f, 100.0f};
        node.InputPins.Add(DialogueGraphPin(GenerateID(), node.ID, "In", PinType::Flow, PinDirection::Input));
        break;

    case NarrativeNodeType::Exit:
        node.Title = title.empty() ? "End Conversation" : title;
        node.Size = {180.0f, 80.0f};
        node.InputPins.Add(DialogueGraphPin(GenerateID(), node.ID, "In", PinType::Flow, PinDirection::Input));
        break;
    }

    m_Nodes.Add(node);
    return &m_Nodes[m_Nodes.Size() - 1];
}

bool DialogueGraph::RemoveNode(uint64_t nodeId)
{
    if (nodeId == m_EntryNodeID)
    {
        // Don't delete entry node if it's the only one
        if (m_Nodes.Size() <= 1)
            return false;
    }

    // Remove all connections attached to this node's pins
    for (size_t i = 0; i < m_Connections.Size();)
    {
        if (m_Connections[i].SourceNodeID == nodeId || m_Connections[i].TargetNodeID == nodeId)
        {
            m_Connections.RemoveAt(i);
        }
        else
        {
            ++i;
        }
    }

    for (size_t i = 0; i < m_Nodes.Size(); ++i)
    {
        if (m_Nodes[i].ID == nodeId)
        {
            m_Nodes.RemoveAt(i);
            if (nodeId == m_EntryNodeID)
            {
                m_EntryNodeID = m_Nodes.Size() > 0 ? m_Nodes[0].ID : 0;
            }
            return true;
        }
    }
    return false;
}

DialogueGraphNode *DialogueGraph::FindNode(uint64_t nodeId)
{
    for (size_t i = 0; i < m_Nodes.Size(); ++i)
    {
        if (m_Nodes[i].ID == nodeId)
            return &m_Nodes[i];
    }
    return nullptr;
}

const DialogueGraphNode *DialogueGraph::FindNode(uint64_t nodeId) const
{
    for (size_t i = 0; i < m_Nodes.Size(); ++i)
    {
        if (m_Nodes[i].ID == nodeId)
            return &m_Nodes[i];
    }
    return nullptr;
}

GraphPin *DialogueGraph::FindPin(uint64_t pinId)
{
    for (size_t n = 0; n < m_Nodes.Size(); ++n)
    {
        for (size_t p = 0; p < m_Nodes[n].InputPins.Size(); ++p)
        {
            if (m_Nodes[n].InputPins[p].ID == pinId)
                return &m_Nodes[n].InputPins[p];
        }
        for (size_t p = 0; p < m_Nodes[n].OutputPins.Size(); ++p)
        {
            if (m_Nodes[n].OutputPins[p].ID == pinId)
                return &m_Nodes[n].OutputPins[p];
        }
    }
    return nullptr;
}

const GraphPin *DialogueGraph::FindPin(uint64_t pinId) const
{
    for (size_t n = 0; n < m_Nodes.Size(); ++n)
    {
        for (size_t p = 0; p < m_Nodes[n].InputPins.Size(); ++p)
        {
            if (m_Nodes[n].InputPins[p].ID == pinId)
                return &m_Nodes[n].InputPins[p];
        }
        for (size_t p = 0; p < m_Nodes[n].OutputPins.Size(); ++p)
        {
            if (m_Nodes[n].OutputPins[p].ID == pinId)
                return &m_Nodes[n].OutputPins[p];
        }
    }
    return nullptr;
}

uint64_t DialogueGraph::Connect(uint64_t srcNodeId, uint64_t srcPinId, uint64_t dstNodeId, uint64_t dstPinId)
{
    if (srcNodeId == dstNodeId)
        return 0; // Prevent direct self-loop

    // Disconnect existing connection from target input pin if single input
    for (size_t i = 0; i < m_Connections.Size(); ++i)
    {
        if (m_Connections[i].TargetPinID == dstPinId)
        {
            m_Connections.RemoveAt(i);
            break;
        }
    }

    uint64_t connId = GenerateID();
    m_Connections.Add(DialogueGraphConnection(connId, srcNodeId, srcPinId, dstNodeId, dstPinId));
    return connId;
}

bool DialogueGraph::Disconnect(uint64_t connectionId)
{
    for (size_t i = 0; i < m_Connections.Size(); ++i)
    {
        if (m_Connections[i].ID == connectionId)
        {
            m_Connections.RemoveAt(i);
            return true;
        }
    }
    return false;
}

bool DialogueGraph::DisconnectPin(uint64_t pinId)
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
            ++i;
        }
    }
    return removed;
}

uint64_t DialogueGraph::GetNextNode(uint64_t currentNodeId, uint64_t pinId) const
{
    for (size_t i = 0; i < m_Connections.Size(); ++i)
    {
        if (m_Connections[i].SourceNodeID == currentNodeId)
        {
            if (pinId == 0 || m_Connections[i].SourcePinID == pinId)
            {
                return m_Connections[i].TargetNodeID;
            }
        }
    }
    return 0;
}

TEArray<DialogueGraphConnection> DialogueGraph::GetConnectionsFromNode(uint64_t nodeId) const
{
    TEArray<DialogueGraphConnection> result;
    for (size_t i = 0; i < m_Connections.Size(); ++i)
    {
        if (m_Connections[i].SourceNodeID == nodeId)
        {
            result.Add(m_Connections[i]);
        }
    }
    return result;
}

void DialogueGraph::Clear()
{
    m_Nodes.Clear();
    m_Connections.Clear();
    m_EntryNodeID = 0;
    m_NextID = 1;
}

void DialogueGraph::ResetToDefaultEntry()
{
    Clear();
    AddNode(NarrativeNodeType::Entry, TEVector2(50.0f, 150.0f), "Story Start");
    auto *diag = AddNode(NarrativeNodeType::Dialogue, TEVector2(320.0f, 150.0f), "Greeting");
    if (m_Nodes.Size() >= 2)
    {
        Connect(m_Nodes[0].ID, m_Nodes[0].OutputPins[0].ID, diag->ID, diag->InputPins[0].ID);
    }
}

