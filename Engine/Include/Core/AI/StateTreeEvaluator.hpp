#pragma once

#include "Core/PreRequisites.h"
#include "Core/AI/StateTreeGraph.hpp"
#include "Core/Scene/Scene.hpp"
#include "GameFrameWork/GameplayUtils.hpp"
#include "Utils/TEString.hpp"


class TE_API StateTreeEvaluator
{
public:
    StateTreeEvaluator() = default;
    ~StateTreeEvaluator();

    void Initialize(TERef<StateTreeGraph> graph, Entity agent);
    void Tick(float dt);
    void Reset();

    uint64_t GetActiveNodeID() const { return m_ActiveNodeID; }
    TEString GetActiveStateName() const;
    bool IsInState(const TEString &name) const;

    const TEArray<TEString> &GetHistory() const { return m_History; }

    Entity GetAgent() const { return m_Agent; }
    TERef<StateTreeGraph> GetGraph() const { return m_Graph; }

private:
    TERef<StateTreeGraph> m_Graph = nullptr;
    Entity m_Agent;
    uint64_t m_ActiveNodeID = 0;
    TERef<StateNode> m_ActiveNode = nullptr;

    float m_StateTime = 0.0f;
    TEArray<TEString> m_History;

    void TransitionTo(uint64_t targetNodeId);
    void EvaluateTransitions();
    void TickActiveTasks(float dt);
};
