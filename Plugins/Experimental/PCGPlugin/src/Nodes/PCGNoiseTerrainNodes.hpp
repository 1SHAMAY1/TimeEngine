#pragma once

#include "Graph/PCGNode.hpp"

class PCGPerlinNoiseNode : public PCGNode
{
public:
    PCGPerlinNoiseNode();
    virtual ~PCGPerlinNoiseNode() override = default;

    virtual bool Execute(PCGExecutionContext &ctx) override;
    virtual TERef<GraphNode> Clone() const override { return CreateRef<PCGPerlinNoiseNode>(*this); }
};

class PCGSimplexNoiseNode : public PCGNode
{
public:
    PCGSimplexNoiseNode();
    virtual ~PCGSimplexNoiseNode() override = default;

    virtual bool Execute(PCGExecutionContext &ctx) override;
    virtual TERef<GraphNode> Clone() const override { return CreateRef<PCGSimplexNoiseNode>(*this); }
};

class PCGWorleyNoiseNode : public PCGNode
{
public:
    PCGWorleyNoiseNode();
    virtual ~PCGWorleyNoiseNode() override = default;

    virtual bool Execute(PCGExecutionContext &ctx) override;
    virtual TERef<GraphNode> Clone() const override { return CreateRef<PCGWorleyNoiseNode>(*this); }
};

class PCGElevationSlopeNode : public PCGNode
{
public:
    PCGElevationSlopeNode();
    virtual ~PCGElevationSlopeNode() override = default;

    virtual bool Execute(PCGExecutionContext &ctx) override;
    virtual TERef<GraphNode> Clone() const override { return CreateRef<PCGElevationSlopeNode>(*this); }
};
