#include "Core/AI/StateTreeEvaluator.hpp"
#include "Core/Log.h"
#include "Core/PreRequisites.h"

StateTreeEvaluator::~StateTreeEvaluator() { Reset(); }

void StateTreeEvaluator::Reset()
{
    if (m_ActiveNode && m_Agent.IsValid())
    {
        for (auto &task : m_ActiveNode->ExitTasks)
        {
            if (task)
                task->ExitState(m_Agent);
        }
    }

    m_ActiveNode = nullptr;
    m_ActiveNodeID = 0;
    m_StateTime = 0.0f;
    m_History.Clear();
}

void StateTreeEvaluator::Initialize(TERef<StateTreeGraph> graph, Entity agent)
{
    Reset();
    m_Graph = graph;
    m_Agent = agent;

    if (!m_Graph)
        return;

    auto initial = m_Graph->GetInitialState();
    if (initial)
    {
        TransitionTo(initial->ID);
    }
}

TEString StateTreeEvaluator::GetActiveStateName() const
{
    if (m_ActiveNode)
        return m_ActiveNode->StateName;
    return "None";
}

bool StateTreeEvaluator::IsInState(const TEString &name) const
{
    if (m_ActiveNode)
        return m_ActiveNode->StateName == name;
    return false;
}

void StateTreeEvaluator::TransitionTo(uint64_t targetNodeId)
{
    if (!m_Graph)
        return;

    auto targetNode = std::dynamic_pointer_cast<StateNode>(m_Graph->FindNode(targetNodeId));
    if (!targetNode)
        return;

    // Exit active state
    if (m_ActiveNode && m_Agent.IsValid())
    {
        for (auto &task : m_ActiveNode->ExitTasks)
        {
            if (task)
                task->ExitState(m_Agent);
        }
    }

    m_ActiveNode = targetNode;
    m_ActiveNodeID = targetNodeId;
    m_StateTime = 0.0f;

    m_History.Add(m_ActiveNode->StateName);
    if (m_History.size() > 20)
    {
        m_History.RemoveAt(0);
    }

    TE_CORE_INFO("[StateTreeEvaluator] Entity {0} transitioned to state '{1}'", m_Agent.IsValid() ? m_Agent.GetID() : 0,
                 m_ActiveNode->StateName.c_str());

    // Enter new state
    if (m_ActiveNode && m_Agent.IsValid())
    {
        for (auto &task : m_ActiveNode->EnterTasks)
        {
            if (task)
                task->EnterState(m_Agent);
        }
    }
}

void StateTreeEvaluator::Tick(float dt)
{
    if (!m_Graph || !m_Agent.IsValid() || !m_ActiveNode)
        return;

    m_StateTime += dt;

    // Evaluate outgoing transitions first
    EvaluateTransitions();

    // Tick active tasks
    TickActiveTasks(dt);
}

void StateTreeEvaluator::EvaluateTransitions()
{
    if (!m_ActiveNode || !m_Graph || !m_Agent.IsValid())
        return;

    if (m_ActiveNode->OutputPins.empty())
        return;

    uint64_t outPinId = m_ActiveNode->OutputPins[0].ID;
    auto conns = m_Graph->GetConnectionsForPin(outPinId);

    for (size_t i = 0; i < conns.size(); ++i)
    {
        const auto &conn = conns[i];
        bool canTransition = true;

        // Check if there is an associated condition
        if (i < m_ActiveNode->TransitionConditions.size() && m_ActiveNode->TransitionConditions[i])
        {
            canTransition = m_ActiveNode->TransitionConditions[i]->Evaluate(m_Agent);
        }

        if (canTransition)
        {
            TransitionTo(conn.TargetNodeID);
            break;
        }
    }
}

void StateTreeEvaluator::TickActiveTasks(float dt)
{
    if (!m_ActiveNode || !m_Agent.IsValid())
        return;

    for (auto &task : m_ActiveNode->TickTasks)
    {
        if (task)
        {
            task->TickState(m_Agent, dt);
        }
    }
}
