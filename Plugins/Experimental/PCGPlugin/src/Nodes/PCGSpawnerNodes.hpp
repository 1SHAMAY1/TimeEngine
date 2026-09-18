#pragma once

#include "Graph/PCGNode.hpp"

class PCGEntitySpawnerNode : public PCGNode
{
public:
    PCGEntitySpawnerNode();
    virtual ~PCGEntitySpawnerNode() override = default;

    virtual bool Execute(PCGExecutionContext &ctx) override;
    virtual TERef<GraphNode> Clone() const override { return CreateRef<PCGEntitySpawnerNode>(*this); }
};

class PCGTilemapSpawnerNode : public PCGNode
{
public:
    PCGTilemapSpawnerNode();
    virtual ~PCGTilemapSpawnerNode() override = default;

    virtual bool Execute(PCGExecutionContext &ctx) override;
    virtual TERef<GraphNode> Clone() const override { return CreateRef<PCGTilemapSpawnerNode>(*this); }
};

class PCGPrototypeAssemblerNode : public PCGNode
{
public:
    PCGPrototypeAssemblerNode();
    virtual ~PCGPrototypeAssemblerNode() override = default;

    virtual bool Execute(PCGExecutionContext &ctx) override;
    virtual TERef<GraphNode> Clone() const override { return CreateRef<PCGPrototypeAssemblerNode>(*this); }
};
