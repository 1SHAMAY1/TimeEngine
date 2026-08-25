#pragma once

#include "Graph/PCGNode.hpp"

// 1. Voronoi Partitioning Node
class PCGVoronoiPartitionNode : public PCGNode
{
public:
    PCGVoronoiPartitionNode();
    virtual ~PCGVoronoiPartitionNode() override = default;
    virtual bool Execute(PCGExecutionContext &ctx) override;
    virtual TERef<GraphNode> Clone() const override { return CreateRef<PCGVoronoiPartitionNode>(*this); }
};

// 2. Delaunay Triangulation Node
class PCGDelaunayTriangulationNode : public PCGNode
{
public:
    PCGDelaunayTriangulationNode();
    virtual ~PCGDelaunayTriangulationNode() override = default;
    virtual bool Execute(PCGExecutionContext &ctx) override;
    virtual TERef<GraphNode> Clone() const override { return CreateRef<PCGDelaunayTriangulationNode>(*this); }
};

// 3. Physics Point Relaxer (XPBD)
class PCGPhysicsPointRelaxerNode : public PCGNode
{
public:
    PCGPhysicsPointRelaxerNode();
    virtual ~PCGPhysicsPointRelaxerNode() override = default;
    virtual bool Execute(PCGExecutionContext &ctx) override;
    virtual TERef<GraphNode> Clone() const override { return CreateRef<PCGPhysicsPointRelaxerNode>(*this); }
};

// 4. Raycast Projection Node
class PCGRaycastProjectionNode : public PCGNode
{
public:
    PCGRaycastProjectionNode();
    virtual ~PCGRaycastProjectionNode() override = default;
    virtual bool Execute(PCGExecutionContext &ctx) override;
    virtual TERef<GraphNode> Clone() const override { return CreateRef<PCGRaycastProjectionNode>(*this); }
};

// 5. Bounds Exclusion Volume Node
class PCGBoundsExclusionNode : public PCGNode
{
public:
    PCGBoundsExclusionNode();
    virtual ~PCGBoundsExclusionNode() override = default;
    virtual bool Execute(PCGExecutionContext &ctx) override;
    virtual TERef<GraphNode> Clone() const override { return CreateRef<PCGBoundsExclusionNode>(*this); }
};

// 6. ECS Component Attribute Injector
class PCGComponentAttributeInjectorNode : public PCGNode
{
public:
    PCGComponentAttributeInjectorNode();
    virtual ~PCGComponentAttributeInjectorNode() override = default;
    virtual bool Execute(PCGExecutionContext &ctx) override;
    virtual TERef<GraphNode> Clone() const override { return CreateRef<PCGComponentAttributeInjectorNode>(*this); }
};

// 7. Entity Assembler & Hierarchy Linker
class PCGEntityHierarchyLinkerNode : public PCGNode
{
public:
    PCGEntityHierarchyLinkerNode();
    virtual ~PCGEntityHierarchyLinkerNode() override = default;
    virtual bool Execute(PCGExecutionContext &ctx) override;
    virtual TERef<GraphNode> Clone() const override { return CreateRef<PCGEntityHierarchyLinkerNode>(*this); }
};

// 8. L-System Structural Generator
class PCGLSystemGeneratorNode : public PCGNode
{
public:
    PCGLSystemGeneratorNode();
    virtual ~PCGLSystemGeneratorNode() override = default;
    virtual bool Execute(PCGExecutionContext &ctx) override;
    virtual TERef<GraphNode> Clone() const override { return CreateRef<PCGLSystemGeneratorNode>(*this); }
};

// 9. Edge/Border Extractor Node
class PCGEdgeExtractorNode : public PCGNode
{
public:
    PCGEdgeExtractorNode();
    virtual ~PCGEdgeExtractorNode() override = default;
    virtual bool Execute(PCGExecutionContext &ctx) override;
    virtual TERef<GraphNode> Clone() const override { return CreateRef<PCGEdgeExtractorNode>(*this); }
};

// 10. Density-Weighted Clustering (K-Means) Node
class PCGKMeansClusteringNode : public PCGNode
{
public:
    PCGKMeansClusteringNode();
    virtual ~PCGKMeansClusteringNode() override = default;
    virtual bool Execute(PCGExecutionContext &ctx) override;
    virtual TERef<GraphNode> Clone() const override { return CreateRef<PCGKMeansClusteringNode>(*this); }
};

// 11. Pathfinding Traversal (A*) Generator
class PCGAStarTraversalNode : public PCGNode
{
public:
    PCGAStarTraversalNode();
    virtual ~PCGAStarTraversalNode() override = default;
    virtual bool Execute(PCGExecutionContext &ctx) override;
    virtual TERef<GraphNode> Clone() const override { return CreateRef<PCGAStarTraversalNode>(*this); }
};

// 12. Component Proximity Mutator
class PCGComponentProximityMutatorNode : public PCGNode
{
public:
    PCGComponentProximityMutatorNode();
    virtual ~PCGComponentProximityMutatorNode() override = default;
    virtual bool Execute(PCGExecutionContext &ctx) override;
    virtual TERef<GraphNode> Clone() const override { return CreateRef<PCGComponentProximityMutatorNode>(*this); }
};
