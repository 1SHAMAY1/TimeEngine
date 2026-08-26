#pragma once

#include "Core/AI/StateTreeConditions.hpp"
#include "Core/AI/StateTreeTasks.hpp"
#include "Core/Graph/Graph.hpp"
#include "Core/Graph/GraphNode.hpp"
#include <memory>
#include <vector>

class TE_API StateNode : public GraphNode
{
public:
    StateNode();
    virtual ~StateNode() override = default;

    TEString StateName = "State";
    TEArray<TERef<IStateTask>> EnterTasks;
    TEArray<TERef<IStateTask>> TickTasks;
    TEArray<TERef<IStateTask>> ExitTasks;

    TEArray<TERef<IStateCondition>> TransitionConditions;

    void AddEnterTask(TERef<IStateTask> task) { EnterTasks.Add(task); }
    void AddTickTask(TERef<IStateTask> task) { TickTasks.Add(task); }
    void AddExitTask(TERef<IStateTask> task) { ExitTasks.Add(task); }

    virtual TERef<GraphNode> Clone() const override;
};

class TE_API StateRootNode : public GraphNode
{
public:
    StateRootNode();
    virtual ~StateRootNode() override = default;

    virtual TERef<GraphNode> Clone() const override;
};

class TE_API StateTreeGraph : public Graph
{
public:
    StateTreeGraph(const TEString &name = "Untitled State Tree");
    virtual ~StateTreeGraph() override = default;

    uint64_t InitialStateNodeID = 0;

    TERef<StateNode> GetInitialState() const;
    TERef<StateNode> FindStateByName(const TEString &name) const;

    TERef<StateNode> CreateState(const TEString &name, const TEVector2 &pos);
    TERef<StateRootNode> CreateRootNode(const TEVector2 &pos);

    bool ConnectStates(uint64_t fromStateNodeId, uint64_t toStateNodeId, TERef<IStateCondition> condition = nullptr);
};
