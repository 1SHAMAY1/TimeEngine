#include "Core/AI/StateTreeGraph.hpp"
#include "Core/PreRequisites.h"

// StateNode
StateNode::StateNode()
{
    Title = "State";
    Category = "AI";
    NodeType = "StateNode";
    Size = {200.0f, 120.0f};
    HeaderColor = TEColor(0.2f, 0.45f, 0.3f, 1.0f);

    // Transition Entry Pin (Input)
    InputPins.Add(GraphPin(0, ID, "In", GraphPinType::Flow(), PinDirection::Input));

    // Transition Out Pin (Output)
    OutputPins.Add(GraphPin(0, ID, "Out", GraphPinType::Flow(), PinDirection::Output));
}

TERef<GraphNode> StateNode::Clone() const
{
    auto copy = CreateRef<StateNode>();
    copy->ID = ID;
    copy->Title = Title;
    copy->Category = Category;
    copy->NodeType = NodeType;
    copy->Position = Position;
    copy->Size = Size;
    copy->HeaderColor = HeaderColor;
    copy->StateName = StateName;
    copy->InputPins = InputPins;
    copy->OutputPins = OutputPins;
    copy->Properties = Properties;

    for (const auto &task : EnterTasks)
        copy->EnterTasks.Add(task ? task->Clone() : nullptr);
    for (const auto &task : TickTasks)
        copy->TickTasks.Add(task ? task->Clone() : nullptr);
    for (const auto &task : ExitTasks)
        copy->ExitTasks.Add(task ? task->Clone() : nullptr);
    for (const auto &cond : TransitionConditions)
        copy->TransitionConditions.Add(cond ? cond->Clone() : nullptr);

    return copy;
}

// StateRootNode
StateRootNode::StateRootNode()
{
    Title = "Root";
    Category = "AI";
    NodeType = "StateRootNode";
    Size = {140.0f, 60.0f};
    HeaderColor = TEColor(0.5f, 0.2f, 0.2f, 1.0f);

    // Initial State Out Pin
    OutputPins.Add(GraphPin(0, ID, "Start", GraphPinType::Flow(), PinDirection::Output));
}

TERef<GraphNode> StateRootNode::Clone() const
{
    auto copy = CreateRef<StateRootNode>();
    copy->ID = ID;
    copy->Title = Title;
    copy->Category = Category;
    copy->NodeType = NodeType;
    copy->Position = Position;
    copy->Size = Size;
    copy->HeaderColor = HeaderColor;
    copy->InputPins = InputPins;
    copy->OutputPins = OutputPins;
    copy->Properties = Properties;
    return copy;
}

// StateTreeGraph
StateTreeGraph::StateTreeGraph(const TEString &name) : Graph(name) {}

TERef<StateNode> StateTreeGraph::GetInitialState() const
{
    if (InitialStateNodeID != 0)
    {
        auto node = FindNode(InitialStateNodeID);
        if (node && node->NodeType == "StateNode")
            return std::dynamic_pointer_cast<StateNode>(node);
    }

    // Otherwise check if RootNode has a connection
    for (const auto &node : m_Nodes)
    {
        if (node && node->NodeType == "StateRootNode")
        {
            if (!node->OutputPins.empty())
            {
                auto conns = GetConnectionsForPin(node->OutputPins[0].ID);
                if (!conns.empty())
                {
                    auto target = FindNode(conns[0].TargetNodeID);
                    if (target && target->NodeType == "StateNode")
                        return std::dynamic_pointer_cast<StateNode>(target);
                }
            }
        }
    }

    // Default to first state node
    for (const auto &node : m_Nodes)
    {
        if (node && node->NodeType == "StateNode")
            return std::dynamic_pointer_cast<StateNode>(node);
    }

    return nullptr;
}

TERef<StateNode> StateTreeGraph::FindStateByName(const TEString &name) const
{
    for (const auto &node : m_Nodes)
    {
        if (node && node->NodeType == "StateNode")
        {
            auto sn = std::dynamic_pointer_cast<StateNode>(node);
            if (sn && sn->StateName == name)
                return sn;
        }
    }
    return nullptr;
}

TERef<StateNode> StateTreeGraph::CreateState(const TEString &name, const TEVector2 &pos)
{
    auto sn = CreateRef<StateNode>();
    sn->ID = GenerateID();
    sn->StateName = name;
    sn->Title = name;
    sn->Position = pos;

    if (!sn->InputPins.empty())
    {
        sn->InputPins[0].ID = GenerateID();
        sn->InputPins[0].NodeID = sn->ID;
    }
    if (!sn->OutputPins.empty())
    {
        sn->OutputPins[0].ID = GenerateID();
        sn->OutputPins[0].NodeID = sn->ID;
    }

    AddNode(sn);
    return sn;
}

TERef<StateRootNode> StateTreeGraph::CreateRootNode(const TEVector2 &pos)
{
    auto root = CreateRef<StateRootNode>();
    root->ID = GenerateID();
    root->Position = pos;

    if (!root->OutputPins.empty())
    {
        root->OutputPins[0].ID = GenerateID();
        root->OutputPins[0].NodeID = root->ID;
    }

    AddNode(root);
    return root;
}

bool StateTreeGraph::ConnectStates(uint64_t fromStateNodeId, uint64_t toStateNodeId, TERef<IStateCondition> condition)
{
    auto fromNode = FindNode(fromStateNodeId);
    auto toNode = FindNode(toStateNodeId);
    if (!fromNode || !toNode)
        return false;

    if (fromNode->OutputPins.empty() || toNode->InputPins.empty())
        return false;

    uint64_t srcPin = fromNode->OutputPins[0].ID;
    uint64_t dstPin = toNode->InputPins[0].ID;

    if (AddConnection(srcPin, dstPin))
    {
        auto sn = std::dynamic_pointer_cast<StateNode>(fromNode);
        if (sn && condition)
        {
            sn->TransitionConditions.Add(condition);
        }
        return true;
    }
    return false;
}
