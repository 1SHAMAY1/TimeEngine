#pragma once

#include "Graph/PCGNode.hpp"

class PCGWilsonMazeNode : public PCGNode
{
public:
    PCGWilsonMazeNode();
    virtual ~PCGWilsonMazeNode() override = default;

    virtual bool Execute(PCGExecutionContext &ctx) override;
    virtual TERef<GraphNode> Clone() const override { return CreateRef<PCGWilsonMazeNode>(*this); }
};

class PCGCellularAutomataNode : public PCGNode
{
public:
    PCGCellularAutomataNode();
    virtual ~PCGCellularAutomataNode() override = default;

    virtual bool Execute(PCGExecutionContext &ctx) override;
    virtual TERef<GraphNode> Clone() const override { return CreateRef<PCGCellularAutomataNode>(*this); }
};

class PCGBSPRoomNode : public PCGNode
{
public:
    PCGBSPRoomNode();
    virtual ~PCGBSPRoomNode() override = default;

    virtual bool Execute(PCGExecutionContext &ctx) override;
    virtual TERef<GraphNode> Clone() const override { return CreateRef<PCGBSPRoomNode>(*this); }
};

class PCGDrunkardWalkNode : public PCGNode
{
public:
    PCGDrunkardWalkNode();
    virtual ~PCGDrunkardWalkNode() override = default;

    virtual bool Execute(PCGExecutionContext &ctx) override;
    virtual TERef<GraphNode> Clone() const override { return CreateRef<PCGDrunkardWalkNode>(*this); }
};

class PCGWaveFunctionCollapseNode : public PCGNode
{
public:
    PCGWaveFunctionCollapseNode();
    virtual ~PCGWaveFunctionCollapseNode() override = default;

    virtual bool Execute(PCGExecutionContext &ctx) override;
    virtual TERef<GraphNode> Clone() const override { return CreateRef<PCGWaveFunctionCollapseNode>(*this); }
};
