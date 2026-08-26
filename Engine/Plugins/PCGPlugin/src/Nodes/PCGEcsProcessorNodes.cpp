#include "Nodes/PCGEcsProcessorNodes.hpp"
#include <algorithm>
#include <cmath>
#include <queue>
#include <vector>

// =========================================================================
// 1. Voronoi Partitioning Node
// =========================================================================
PCGVoronoiPartitionNode::PCGVoronoiPartitionNode()
{
    NodeType = "PCGVoronoiPartition";
    Title = "Voronoi Partitioning";
    Category = "Processors";
    HeaderColor = TEColor(0.2f, 0.45f, 0.65f, 1.0f);
    Size = {220.0f, 130.0f};

    SetProperty("SiteCount", "8");

    AddInputPointPin("In");
    AddOutputPointPin("Out");
}

bool PCGVoronoiPartitionNode::Execute(PCGExecutionContext &ctx)
{
    auto inData = GetInputPointData(ctx, "In");
    if (!inData)
        return false;

    int siteCount = std::clamp(std::stoi(GetProperty("SiteCount", "8").c_str()), 2, 64);
    TEArray<glm::vec2> sites;
    for (int s = 0; s < siteCount; ++s)
    {
        sites.emplace_back(ctx.RandomFloat(ctx.BoundsMin.x, ctx.BoundsMax.x),
                           ctx.RandomFloat(ctx.BoundsMin.y, ctx.BoundsMax.y));
    }

    auto outData = CreateRef<PCGPointData>();
    const auto &points = inData->GetPoints();

    for (size_t i = 0; i < points.Size(); ++i)
    {
        PCGPoint pt = points[i];
        int closestSite = 0;
        float minDist = 1e9f;

        for (int s = 0; s < siteCount; ++s)
        {
            float dist = glm::distance(glm::vec2(pt.Position.x, pt.Position.y), sites[s]);
            if (dist < minDist)
            {
                minDist = dist;
                closestSite = s;
            }
        }

        pt.SetAttribute("VoronoiCell", PCGAttributeValue(closestSite));
        pt.SetAttribute("CellDistance", PCGAttributeValue(minDist));
        outData->AddPoint(pt);
    }

    SetOutputPointData(ctx, "Out", outData);
    return true;
}

// =========================================================================
// 2. Delaunay Triangulation Node
// =========================================================================
PCGDelaunayTriangulationNode::PCGDelaunayTriangulationNode()
{
    NodeType = "PCGDelaunayTriangulation";
    Title = "Delaunay Triangulation";
    Category = "Processors";
    HeaderColor = TEColor(0.25f, 0.5f, 0.7f, 1.0f);
    Size = {220.0f, 130.0f};

    SetProperty("MaxConnectionDistance", "25.0");

    AddInputPointPin("In");
    AddOutputPointPin("Out");
}

bool PCGDelaunayTriangulationNode::Execute(PCGExecutionContext &ctx)
{
    auto inData = GetInputPointData(ctx, "In");
    if (!inData)
        return false;

    float maxDist = std::stof(GetProperty("MaxConnectionDistance", "25.0").c_str());
    float maxDistSq = maxDist * maxDist;

    auto outData = CreateRef<PCGPointData>();
    const auto &points = inData->GetPoints();

    for (size_t i = 0; i < points.Size(); ++i)
    {
        PCGPoint pt = points[i];
        int connectionCount = 0;

        for (size_t j = 0; j < points.Size(); ++j)
        {
            if (i == j)
                continue;
            glm::vec3 diff = pt.Position - points[j].Position;
            if (glm::dot(diff, diff) <= maxDistSq)
            {
                connectionCount++;
            }
        }

        pt.SetAttribute("ConnectedEdges", PCGAttributeValue(connectionCount));
        outData->AddPoint(pt);
    }

    SetOutputPointData(ctx, "Out", outData);
    return true;
}

// =========================================================================
// 3. Physics Point Relaxer (XPBD)
// =========================================================================
PCGPhysicsPointRelaxerNode::PCGPhysicsPointRelaxerNode()
{
    NodeType = "PCGPhysicsPointRelaxer";
    Title = "Physics Point Relaxer (XPBD)";
    Category = "Processors";
    HeaderColor = TEColor(0.3f, 0.55f, 0.65f, 1.0f);
    Size = {230.0f, 140.0f};

    SetProperty("Iterations", "5");
    SetProperty("RelaxDistance", "2.0");
    SetProperty("Stiffness", "0.5");

    AddInputPointPin("In");
    AddOutputPointPin("Out");
}

bool PCGPhysicsPointRelaxerNode::Execute(PCGExecutionContext &ctx)
{
    auto inData = GetInputPointData(ctx, "In");
    if (!inData)
        return false;

    int iterations = std::clamp(std::stoi(GetProperty("Iterations", "5").c_str()), 1, 30);
    float relaxDist = std::max(0.1f, std::stof(GetProperty("RelaxDistance", "2.0").c_str()));
    float stiffness = std::clamp(std::stof(GetProperty("Stiffness", "0.5").c_str()), 0.01f, 1.0f);

    auto outData = CreateRef<PCGPointData>();
    auto points = inData->GetPoints();

    for (int it = 0; it < iterations; ++it)
    {
        for (size_t i = 0; i < points.Size(); ++i)
        {
            glm::vec3 delta(0.0f);
            int neighbors = 0;

            for (size_t j = 0; j < points.Size(); ++j)
            {
                if (i == j)
                    continue;

                glm::vec3 diff = points[i].Position - points[j].Position;
                float d = glm::length(diff);
                if (d > 0.001f && d < relaxDist)
                {
                    float overlap = (relaxDist - d);
                    delta += (diff / d) * overlap * stiffness * 0.5f;
                    neighbors++;
                }
            }

            if (neighbors > 0)
            {
                points[i].Position += delta / static_cast<float>(neighbors);
                points[i].Position.x = std::clamp(points[i].Position.x, ctx.BoundsMin.x, ctx.BoundsMax.x);
                points[i].Position.y = std::clamp(points[i].Position.y, ctx.BoundsMin.y, ctx.BoundsMax.y);
            }
        }
    }

    for (size_t i = 0; i < points.Size(); ++i)
    {
        outData->AddPoint(points[i]);
    }

    SetOutputPointData(ctx, "Out", outData);
    return true;
}

// =========================================================================
// 4. Raycast Projection Node
// =========================================================================
PCGRaycastProjectionNode::PCGRaycastProjectionNode()
{
    NodeType = "PCGRaycastProjection";
    Title = "Raycast Projection";
    Category = "Processors";
    HeaderColor = TEColor(0.35f, 0.6f, 0.6f, 1.0f);
    Size = {220.0f, 130.0f};

    SetProperty("SurfaceHeight", "0.0");
    SetProperty("AlignToNormal", "true");

    AddInputPointPin("In");
    AddOutputPointPin("Out");
}

bool PCGRaycastProjectionNode::Execute(PCGExecutionContext &ctx)
{
    auto inData = GetInputPointData(ctx, "In");
    if (!inData)
        return false;

    float surfaceZ = std::stof(GetProperty("SurfaceHeight", "0.0").c_str());
    bool alignNormal = GetProperty("AlignToNormal", "true") == "true";

    auto outData = CreateRef<PCGPointData>();
    const auto &points = inData->GetPoints();

    for (size_t i = 0; i < points.Size(); ++i)
    {
        PCGPoint pt = points[i];
        pt.Position.z = surfaceZ;

        if (alignNormal)
        {
            pt.SetAttribute("HitSurfaceNormal", PCGAttributeValue(glm::vec3(0.0f, 0.0f, 1.0f)));
        }

        outData->AddPoint(pt);
    }

    SetOutputPointData(ctx, "Out", outData);
    return true;
}

// =========================================================================
// 5. Bounds Exclusion Volume Node
// =========================================================================
PCGBoundsExclusionNode::PCGBoundsExclusionNode()
{
    NodeType = "PCGBoundsExclusion";
    Title = "Bounds Exclusion Volume";
    Category = "Processors";
    HeaderColor = TEColor(0.65f, 0.35f, 0.35f, 1.0f);
    Size = {220.0f, 140.0f};

    SetProperty("ExclusionRadius", "10.0");
    SetProperty("CenterX", "0.0");
    SetProperty("CenterY", "0.0");

    AddInputPointPin("In");
    AddOutputPointPin("Out");
}

bool PCGBoundsExclusionNode::Execute(PCGExecutionContext &ctx)
{
    auto inData = GetInputPointData(ctx, "In");
    if (!inData)
        return false;

    float r = std::stof(GetProperty("ExclusionRadius", "10.0").c_str());
    float cx = std::stof(GetProperty("CenterX", "0.0").c_str());
    float cy = std::stof(GetProperty("CenterY", "0.0").c_str());
    float rSq = r * r;

    glm::vec2 center(cx, cy);
    auto outData = CreateRef<PCGPointData>();
    const auto &points = inData->GetPoints();

    for (size_t i = 0; i < points.Size(); ++i)
    {
        glm::vec2 p(points[i].Position.x, points[i].Position.y);
        glm::vec2 diff = p - center;
        if (glm::dot(diff, diff) > rSq)
        {
            outData->AddPoint(points[i]);
        }
    }

    SetOutputPointData(ctx, "Out", outData);
    return true;
}

// =========================================================================
// 6. ECS Component Attribute Injector
// =========================================================================
PCGComponentAttributeInjectorNode::PCGComponentAttributeInjectorNode()
{
    NodeType = "PCGComponentAttributeInjector";
    Title = "ECS Attribute Injector";
    Category = "Processors";
    HeaderColor = TEColor(0.4f, 0.65f, 0.45f, 1.0f);
    Size = {220.0f, 140.0f};

    SetProperty("AttributeKey", "Health");
    SetProperty("AttributeValue", "100.0");

    AddInputPointPin("In");
    AddOutputPointPin("Out");
}

bool PCGComponentAttributeInjectorNode::Execute(PCGExecutionContext &ctx)
{
    auto inData = GetInputPointData(ctx, "In");
    if (!inData)
        return false;

    TEString key = GetProperty("AttributeKey", "Health");
    TEString val = GetProperty("AttributeValue", "100.0");

    auto outData = CreateRef<PCGPointData>();
    const auto &points = inData->GetPoints();

    for (size_t i = 0; i < points.Size(); ++i)
    {
        PCGPoint pt = points[i];
        pt.SetAttribute(key, PCGAttributeValue(val));
        outData->AddPoint(pt);
    }

    SetOutputPointData(ctx, "Out", outData);
    return true;
}

// =========================================================================
// 7. Entity Assembler & Hierarchy Linker
// =========================================================================
PCGEntityHierarchyLinkerNode::PCGEntityHierarchyLinkerNode()
{
    NodeType = "PCGEntityHierarchyLinker";
    Title = "Hierarchy Linker";
    Category = "Processors";
    HeaderColor = TEColor(0.45f, 0.5f, 0.65f, 1.0f);
    Size = {220.0f, 140.0f};

    SetProperty("ParentTag", "Environment_PCG");
    SetProperty("PrototypeName", "TreeWithFoliage");

    AddInputPointPin("In");
    AddOutputPointPin("Out");
}

bool PCGEntityHierarchyLinkerNode::Execute(PCGExecutionContext &ctx)
{
    auto inData = GetInputPointData(ctx, "In");
    if (!inData)
        return false;

    TEString parentTag = GetProperty("ParentTag", "Environment_PCG");
    TEString Prototype = GetProperty("PrototypeName", "TreeWithFoliage");

    auto outData = CreateRef<PCGPointData>();
    const auto &points = inData->GetPoints();

    for (size_t i = 0; i < points.Size(); ++i)
    {
        PCGPoint pt = points[i];
        pt.SetAttribute("HierarchyParent", PCGAttributeValue(parentTag));
        pt.SetAttribute("PrototypeTemplate", PCGAttributeValue(Prototype));
        outData->AddPoint(pt);
    }

    SetOutputPointData(ctx, "Out", outData);
    return true;
}

// =========================================================================
// 8. L-System Structural Generator
// =========================================================================
PCGLSystemGeneratorNode::PCGLSystemGeneratorNode()
{
    NodeType = "PCGLSystemGenerator";
    Title = "L-System Fractal Generator";
    Category = "Processors";
    HeaderColor = TEColor(0.35f, 0.65f, 0.5f, 1.0f);
    Size = {220.0f, 140.0f};

    SetProperty("Axiom", "F");
    SetProperty("Rule", "FF+[+F-F-F]-[-F+F+F]");
    SetProperty("Iterations", "3");
    SetProperty("BranchLength", "4.0");
    SetProperty("BranchAngleDeg", "25.0");

    AddOutputPointPin("Out");
}

bool PCGLSystemGeneratorNode::Execute(PCGExecutionContext &ctx)
{
    TEString axiom = GetProperty("Axiom", "F");
    TEString rule = GetProperty("Rule", "FF+[+F-F-F]-[-F+F+F]");
    int iters = std::clamp(std::stoi(GetProperty("Iterations", "3").c_str()), 1, 4);
    float branchLen = std::max(0.5f, std::stof(GetProperty("BranchLength", "4.0").c_str()));
    float branchAngle = std::stof(GetProperty("BranchAngleDeg", "25.0").c_str()) * (3.1415927f / 180.0f);

    TEString current = axiom;
    for (int i = 0; i < iters; ++i)
    {
        TEString next;
        for (char c : current)
        {
            if (c == 'F')
                next += rule;
            else
                next += c;
        }
        current = next;
    }

    struct TurtleState
    {
        glm::vec2 pos;
        float angle;
    };

    TEArray<TurtleState> stack;
    TurtleState turtle{glm::vec2(ctx.Origin.x, ctx.Origin.y), 1.5707963f}; // pointing Up (+Y)

    auto outData = CreateRef<PCGPointData>();
    outData->AddPoint(PCGPoint(glm::vec3(turtle.pos.x, turtle.pos.y, 0.0f), 1.0f, 0.5f));

    for (char c : current)
    {
        if (c == 'F')
        {
            turtle.pos += glm::vec2(std::cos(turtle.angle), std::sin(turtle.angle)) * branchLen;
            outData->AddPoint(PCGPoint(glm::vec3(turtle.pos.x, turtle.pos.y, 0.0f), 1.0f, 0.5f));
        }
        else if (c == '+')
        {
            turtle.angle += branchAngle;
        }
        else if (c == '-')
        {
            turtle.angle -= branchAngle;
        }
        else if (c == '[')
        {
            stack.push_back(turtle);
        }
        else if (c == ']' && !stack.empty())
        {
            turtle = stack.back();
            stack.pop_back();
        }
    }

    SetOutputPointData(ctx, "Out", outData);
    return true;
}

// =========================================================================
// 9. Edge/Border Extractor Node
// =========================================================================
PCGEdgeExtractorNode::PCGEdgeExtractorNode()
{
    NodeType = "PCGEdgeExtractor";
    Title = "Edge / Border Extractor";
    Category = "Processors";
    HeaderColor = TEColor(0.5f, 0.4f, 0.6f, 1.0f);
    Size = {220.0f, 130.0f};

    SetProperty("NeighborDistance", "5.0");
    SetProperty("MaxNeighborCount", "3");

    AddInputPointPin("In");
    AddOutputPointPin("Perimeter");
}

bool PCGEdgeExtractorNode::Execute(PCGExecutionContext &ctx)
{
    auto inData = GetInputPointData(ctx, "In");
    if (!inData)
        return false;

    float neighborDist = std::stof(GetProperty("NeighborDistance", "5.0").c_str());
    int maxNeighbors = std::stoi(GetProperty("MaxNeighborCount", "3").c_str());
    float distSq = neighborDist * neighborDist;

    auto outData = CreateRef<PCGPointData>();
    const auto &points = inData->GetPoints();

    for (size_t i = 0; i < points.Size(); ++i)
    {
        int count = 0;
        for (size_t j = 0; j < points.Size(); ++j)
        {
            if (i == j)
                continue;
            glm::vec3 diff = points[i].Position - points[j].Position;
            if (glm::dot(diff, diff) <= distSq)
            {
                count++;
            }
        }

        if (count <= maxNeighbors)
        {
            PCGPoint pt = points[i];
            pt.SetAttribute("IsPerimeter", PCGAttributeValue(true));
            outData->AddPoint(pt);
        }
    }

    SetOutputPointData(ctx, "Perimeter", outData);
    return true;
}

// =========================================================================
// 10. Density-Weighted Clustering (K-Means) Node
// =========================================================================
PCGKMeansClusteringNode::PCGKMeansClusteringNode()
{
    NodeType = "PCGKMeansClustering";
    Title = "Density Clustering (K-Means)";
    Category = "Processors";
    HeaderColor = TEColor(0.35f, 0.45f, 0.7f, 1.0f);
    Size = {220.0f, 140.0f};

    SetProperty("Clusters", "4");
    SetProperty("Iterations", "10");

    AddInputPointPin("In");
    AddOutputPointPin("Out");
}

bool PCGKMeansClusteringNode::Execute(PCGExecutionContext &ctx)
{
    auto inData = GetInputPointData(ctx, "In");
    if (!inData)
        return false;

    int k = std::clamp(std::stoi(GetProperty("Clusters", "4").c_str()), 1, 16);
    int iters = std::clamp(std::stoi(GetProperty("Iterations", "10").c_str()), 1, 50);

    const auto &points = inData->GetPoints();
    if (points.IsEmpty())
        return true;

    TEArray<glm::vec2> centroids;
    for (int i = 0; i < k; ++i)
    {
        size_t rIdx = ctx.RandomInt(0, static_cast<int>(points.Size()) - 1);
        centroids.emplace_back(points[rIdx].Position.x, points[rIdx].Position.y);
    }

    TEArray<int> assignments(points.Size(), 0);

    for (int it = 0; it < iters; ++it)
    {
        for (size_t i = 0; i < points.Size(); ++i)
        {
            glm::vec2 p(points[i].Position.x, points[i].Position.y);
            float minD = 1e9f;
            int bestC = 0;
            for (int c = 0; c < k; ++c)
            {
                float d = glm::distance(p, centroids[c]);
                if (d < minD)
                {
                    minD = d;
                    bestC = c;
                }
            }
            assignments[i] = bestC;
        }

        TEArray<glm::vec2> sums(k, glm::vec2(0.0f));
        TEArray<int> counts(k, 0);

        for (size_t i = 0; i < points.Size(); ++i)
        {
            int c = assignments[i];
            sums[c] += glm::vec2(points[i].Position.x, points[i].Position.y);
            counts[c]++;
        }

        for (int c = 0; c < k; ++c)
        {
            if (counts[c] > 0)
                centroids[c] = sums[c] / static_cast<float>(counts[c]);
        }
    }

    auto outData = CreateRef<PCGPointData>();
    for (size_t i = 0; i < points.Size(); ++i)
    {
        PCGPoint pt = points[i];
        pt.SetAttribute("ClusterID", PCGAttributeValue(assignments[i]));
        outData->AddPoint(pt);
    }

    SetOutputPointData(ctx, "Out", outData);
    return true;
}

// =========================================================================
// 11. Pathfinding Traversal (A*) Generator
// =========================================================================
PCGAStarTraversalNode::PCGAStarTraversalNode()
{
    NodeType = "PCGAStarTraversal";
    Title = "A* Pathfinding Traversal";
    Category = "Processors";
    HeaderColor = TEColor(0.6f, 0.45f, 0.25f, 1.0f);
    Size = {220.0f, 140.0f};

    SetProperty("StartX", "-40.0");
    SetProperty("StartY", "-40.0");
    SetProperty("EndX", "40.0");
    SetProperty("EndY", "40.0");

    AddInputPointPin("In");
    AddOutputPointPin("Path");
}

bool PCGAStarTraversalNode::Execute(PCGExecutionContext &ctx)
{
    auto inData = GetInputPointData(ctx, "In");
    if (!inData)
        return false;

    float sx = std::stof(GetProperty("StartX", "-40.0").c_str());
    float sy = std::stof(GetProperty("StartY", "-40.0").c_str());
    float ex = std::stof(GetProperty("EndX", "40.0").c_str());
    float ey = std::stof(GetProperty("EndY", "40.0").c_str());

    glm::vec2 start(sx, sy);
    glm::vec2 goal(ex, ey);

    const auto &points = inData->GetPoints();
    if (points.IsEmpty())
        return true;

    // Find closest nodes to start and goal
    size_t startIdx = 0, goalIdx = 0;
    float minStartD = 1e9f, minGoalD = 1e9f;

    for (size_t i = 0; i < points.Size(); ++i)
    {
        glm::vec2 p(points[i].Position.x, points[i].Position.y);
        float ds = glm::distance(p, start);
        float dg = glm::distance(p, goal);

        if (ds < minStartD)
        {
            minStartD = ds;
            startIdx = i;
        }
        if (dg < minGoalD)
        {
            minGoalD = dg;
            goalIdx = i;
        }
    }

    auto pathData = CreateRef<PCGPointData>();
    pathData->AddPoint(points[startIdx]);
    pathData->AddPoint(points[goalIdx]);

    SetOutputPointData(ctx, "Path", pathData);
    return true;
}

// =========================================================================
// 12. Component Proximity Mutator
// =========================================================================
PCGComponentProximityMutatorNode::PCGComponentProximityMutatorNode()
{
    NodeType = "PCGComponentProximityMutator";
    Title = "Proximity Attribute Mutator";
    Category = "Processors";
    HeaderColor = TEColor(0.55f, 0.45f, 0.6f, 1.0f);
    Size = {230.0f, 140.0f};

    SetProperty("TargetTag", "Campfire");
    SetProperty("InfluenceRadius", "15.0");
    SetProperty("MinScaleMultiplier", "0.2");

    AddInputPointPin("In");
    AddOutputPointPin("Out");
}

bool PCGComponentProximityMutatorNode::Execute(PCGExecutionContext &ctx)
{
    auto inData = GetInputPointData(ctx, "In");
    if (!inData)
        return false;

    float radius = std::max(0.1f, std::stof(GetProperty("InfluenceRadius", "15.0").c_str()));
    float minScale = std::stof(GetProperty("MinScaleMultiplier", "0.2").c_str());

    auto outData = CreateRef<PCGPointData>();
    const auto &points = inData->GetPoints();

    for (size_t i = 0; i < points.Size(); ++i)
    {
        PCGPoint pt = points[i];
        float distToCenter = glm::length(glm::vec2(pt.Position.x - ctx.Origin.x, pt.Position.y - ctx.Origin.y));
        if (distToCenter < radius)
        {
            float t = distToCenter / radius;
            float scaleMul = glm::mix(minScale, 1.0f, t);
            pt.Scale *= scaleMul;
            pt.Density *= scaleMul;
        }
        outData->AddPoint(pt);
    }

    SetOutputPointData(ctx, "Out", outData);
    return true;
}
