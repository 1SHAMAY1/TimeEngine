#pragma once

#include "Graph/PCGNode.hpp"

class PCGGridGeneratorNode : public PCGNode
{
public:
    PCGGridGeneratorNode();
    virtual ~PCGGridGeneratorNode() override = default;

    virtual bool Execute(PCGExecutionContext &ctx) override;
    virtual TERef<GraphNode> Clone() const override { return CreateRef<PCGGridGeneratorNode>(*this); }
};

class PCGRandomScatterNode : public PCGNode
{
public:
    PCGRandomScatterNode();
    virtual ~PCGRandomScatterNode() override = default;

    virtual bool Execute(PCGExecutionContext &ctx) override;
    virtual TERef<GraphNode> Clone() const override { return CreateRef<PCGRandomScatterNode>(*this); }
};

class PCGPoissonDiskNode : public PCGNode
{
public:
    PCGPoissonDiskNode();
    virtual ~PCGPoissonDiskNode() override = default;

    virtual bool Execute(PCGExecutionContext &ctx) override;
    virtual TERef<GraphNode> Clone() const override { return CreateRef<PCGPoissonDiskNode>(*this); }
};

class PCGSplinePathNode : public PCGNode
{
public:
    PCGSplinePathNode();
    virtual ~PCGSplinePathNode() override = default;

    virtual bool Execute(PCGExecutionContext &ctx) override;
    virtual TERef<GraphNode> Clone() const override { return CreateRef<PCGSplinePathNode>(*this); }
};
