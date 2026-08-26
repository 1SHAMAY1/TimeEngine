#include "Asset/PCGGraphSerializer.hpp"
#include "Core/Log.h"
#include "Nodes/PCGEcsProcessorNodes.hpp"
#include "Nodes/PCGGeneratorNodes.hpp"
#include "Nodes/PCGNoiseTerrainNodes.hpp"
#include "Nodes/PCGSpawnerNodes.hpp"
#include "Nodes/PCGStructuralNodes.hpp"
#include "Utils/TEFileSystem.hpp"
#include <fstream>

static TERef<GraphNode> CreateNodeByType(const TEString &type)
{
    if (type == "PCGGridGenerator")
        return CreateRef<PCGGridGeneratorNode>();
    if (type == "PCGRandomScatter")
        return CreateRef<PCGRandomScatterNode>();
    if (type == "PCGPoissonDisk")
        return CreateRef<PCGPoissonDiskNode>();
    if (type == "PCGSplinePath")
        return CreateRef<PCGSplinePathNode>();

    if (type == "PCGPerlinNoise")
        return CreateRef<PCGPerlinNoiseNode>();
    if (type == "PCGSimplexNoise")
        return CreateRef<PCGSimplexNoiseNode>();
    if (type == "PCGWorleyNoise")
        return CreateRef<PCGWorleyNoiseNode>();
    if (type == "PCGElevationSlope")
        return CreateRef<PCGElevationSlopeNode>();

    if (type == "PCGWilsonMaze")
        return CreateRef<PCGWilsonMazeNode>();
    if (type == "PCGCellularAutomata")
        return CreateRef<PCGCellularAutomataNode>();
    if (type == "PCGBSPRoom")
        return CreateRef<PCGBSPRoomNode>();
    if (type == "PCGDrunkardWalk")
        return CreateRef<PCGDrunkardWalkNode>();
    if (type == "PCGWaveFunctionCollapse")
        return CreateRef<PCGWaveFunctionCollapseNode>();

    if (type == "PCGVoronoiPartition")
        return CreateRef<PCGVoronoiPartitionNode>();
    if (type == "PCGDelaunayTriangulation")
        return CreateRef<PCGDelaunayTriangulationNode>();
    if (type == "PCGPhysicsPointRelaxer")
        return CreateRef<PCGPhysicsPointRelaxerNode>();
    if (type == "PCGRaycastProjection")
        return CreateRef<PCGRaycastProjectionNode>();
    if (type == "PCGBoundsExclusion")
        return CreateRef<PCGBoundsExclusionNode>();
    if (type == "PCGComponentAttributeInjector")
        return CreateRef<PCGComponentAttributeInjectorNode>();
    if (type == "PCGEntityHierarchyLinker")
        return CreateRef<PCGEntityHierarchyLinkerNode>();
    if (type == "PCGLSystemGenerator")
        return CreateRef<PCGLSystemGeneratorNode>();
    if (type == "PCGEdgeExtractor")
        return CreateRef<PCGEdgeExtractorNode>();
    if (type == "PCGKMeansClustering")
        return CreateRef<PCGKMeansClusteringNode>();
    if (type == "PCGAStarTraversal")
        return CreateRef<PCGAStarTraversalNode>();
    if (type == "PCGComponentProximityMutator")
        return CreateRef<PCGComponentProximityMutatorNode>();

    if (type == "PCGEntitySpawner")
        return CreateRef<PCGEntitySpawnerNode>();
    if (type == "PCGTilemapSpawner")
        return CreateRef<PCGTilemapSpawnerNode>();
    if (type == "PCGPrototypeAssembler")
        return CreateRef<PCGPrototypeAssemblerNode>();

    return nullptr;
}

bool PCGGraphSerializer::SaveToFile(const PCGGraph &graph, const TEString &path)
{
    std::ofstream file(path.c_str());
    if (!file.is_open())
    {
        TE_CORE_ERROR("[PCGGraphSerializer] Failed to open file for writing: {0}", path);
        return false;
    }

    file << "# TimeEngine PCG Graph Asset\n";
    file << "Version: 1.0\n";
    file << "Name: " << graph.GetName().c_str() << "\n";
    file << "NextID: " << graph.GetNextID() << "\n\n";

    // Write Nodes
    const auto &nodes = graph.GetNodes();
    file << "[Nodes] " << nodes.Size() << "\n";
    for (size_t i = 0; i < nodes.Size(); ++i)
    {
        const auto &node = nodes[i];
        if (!node)
            continue;

        file << "Node: " << node->ID << " | " << node->NodeType.c_str() << " | " << node->Position.x << " "
             << node->Position.y << " | " << node->Title.c_str() << "\n";

        // Properties
        for (auto it = node->Properties.begin(); it != node->Properties.end(); ++it)
        {
            file << "  Prop: " << it->first.c_str() << "=" << it->second.c_str() << "\n";
        }

        // Input Pins
        for (size_t p = 0; p < node->InputPins.Size(); ++p)
        {
            const auto &pin = node->InputPins[p];
            file << "  InPin: " << pin.ID << " | " << pin.Name.c_str() << "\n";
        }

        // Output Pins
        for (size_t p = 0; p < node->OutputPins.Size(); ++p)
        {
            const auto &pin = node->OutputPins[p];
            file << "  OutPin: " << pin.ID << " | " << pin.Name.c_str() << "\n";
        }
    }

    // Write Connections
    const auto &connections = graph.GetConnections();
    file << "\n[Connections] " << connections.Size() << "\n";
    for (size_t i = 0; i < connections.Size(); ++i)
    {
        const auto &c = connections[i];
        file << "Conn: " << c.ID << " | " << c.SourcePinID << " -> " << c.TargetPinID << "\n";
    }

    return true;
}

bool PCGGraphSerializer::LoadFromFile(PCGGraph &graph, const TEString &path)
{
    if (!TEFileSystem::Exists(path))
    {
        TE_CORE_ERROR("[PCGGraphSerializer] Failed to open file for reading: {0}", path);
        return false;
    }

    graph.Clear();

    TEString text = TEFileSystem::ReadAllText(path);
    TEArray<TEString> lines = text.Split('\n');
    TERef<GraphNode> currentNode = nullptr;

    for (const auto &rawLine : lines)
    {
        TEString line = rawLine.Trim();
        if (line.IsEmpty() || line.StartsWith("#"))
            continue;

        if (line.StartsWith("Name: "))
        {
            graph.SetName(line.Mid(6));
        }
        else if (line.StartsWith("NextID: "))
        {
            graph.SetNextID(static_cast<uint64_t>(line.Mid(8).ToInt64()));
        }
        else if (line.StartsWith("Node: "))
        {
            // Node: ID | Type | PosX PosY | Title
            TEString content = line.Mid(6);
            auto parts = content.Split('|');
            if (parts.Num() >= 4)
            {
                uint64_t nodeId = static_cast<uint64_t>(parts[0].Trim().ToInt64());
                TEString typeStr = parts[1].Trim();
                TEString posStr = parts[2].Trim();
                TEString titleStr = parts[3].Trim();

                float px = 0.0f, py = 0.0f;
                auto posParts = posStr.Split(' ');
                if (posParts.Num() >= 2)
                {
                    px = posParts[0].ToFloat();
                    py = posParts[1].ToFloat();
                }

                auto node = CreateNodeByType(typeStr);
                if (node)
                {
                    node->ID = nodeId;
                    node->Position = {px, py};
                    if (!titleStr.IsEmpty())
                        node->Title = titleStr;

                    graph.AddNode(node);
                    currentNode = node;
                }
            }
        }
        else if (line.StartsWith("Prop: ") && currentNode)
        {
            TEString prop = line.Mid(6);
            int eq = prop.Find("=");
            if (eq >= 0)
            {
                TEString k = prop.Left(eq);
                TEString v = prop.Mid(eq + 1);
                currentNode->SetProperty(k.c_str(), v.c_str());
            }
        }
        else if (line.StartsWith("InPin: ") && currentNode)
        {
            TEString pinContent = line.Mid(7);
            auto parts = pinContent.Split('|');
            if (parts.Num() >= 2)
            {
                uint64_t pinId = static_cast<uint64_t>(parts[0].Trim().ToInt64());
                TEString pinName = parts[1].Trim();
                auto *p = currentNode->FindPinByName(pinName.c_str(), PinDirection::Input);
                if (p)
                    p->ID = pinId;
            }
        }
        else if (line.StartsWith("OutPin: ") && currentNode)
        {
            TEString pinContent = line.Mid(8);
            auto parts = pinContent.Split('|');
            if (parts.Num() >= 2)
            {
                uint64_t pinId = static_cast<uint64_t>(parts[0].Trim().ToInt64());
                TEString pinName = parts[1].Trim();
                auto *p = currentNode->FindPinByName(pinName.c_str(), PinDirection::Output);
                if (p)
                    p->ID = pinId;
            }
        }
        else if (line.StartsWith("Conn: "))
        {
            // Conn: ID | SrcPinID -> DstPinID
            TEString connContent = line.Mid(6);
            auto parts = connContent.Split('|');
            if (parts.Num() >= 2)
            {
                auto connParts = parts[1].Split("->");
                if (connParts.Num() >= 2)
                {
                    uint64_t srcId = static_cast<uint64_t>(connParts[0].Trim().ToInt64());
                    uint64_t dstId = static_cast<uint64_t>(connParts[1].Trim().ToInt64());
                    graph.AddConnection(srcId, dstId);
                }
            }
        }
    }

    return true;
}
