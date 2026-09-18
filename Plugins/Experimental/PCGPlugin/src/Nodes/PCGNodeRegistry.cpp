#include "Nodes/PCGNodeRegistry.hpp"
#include "Nodes/PCGEcsProcessorNodes.hpp"
#include "Nodes/PCGGeneratorNodes.hpp"
#include "Nodes/PCGNoiseTerrainNodes.hpp"
#include "Nodes/PCGSpawnerNodes.hpp"
#include "Nodes/PCGStructuralNodes.hpp"

void PCGNodeRegistry::RegisterAllNodes(NodePalettePopup &palette)
{
    // Generators
    palette.RegisterNode("Grid Generator", "Generators", "PCGGridGenerator", "Generates points along a regular 2D grid",
                         []() -> TERef<GraphNode> { return CreateRef<PCGGridGeneratorNode>(); });
    palette.RegisterNode("Random Scatter", "Generators", "PCGRandomScatter", "Scatters points randomly within bounds",
                         []() -> TERef<GraphNode> { return CreateRef<PCGRandomScatterNode>(); });
    palette.RegisterNode("Poisson Disk Sampler", "Generators", "PCGPoissonDisk",
                         "Generates blue-noise points with minimum distance spacing",
                         []() -> TERef<GraphNode> { return CreateRef<PCGPoissonDiskNode>(); });
    palette.RegisterNode("Spline / Path Sampler", "Generators", "PCGSplinePath",
                         "Samples points along a bezier spline path",
                         []() -> TERef<GraphNode> { return CreateRef<PCGSplinePathNode>(); });

    // Noise & Terrain
    palette.RegisterNode("Perlin Noise Filter", "Noise & Terrain", "PCGPerlinNoise",
                         "Filters or modulates point density with Perlin noise",
                         []() -> TERef<GraphNode> { return CreateRef<PCGPerlinNoiseNode>(); });
    palette.RegisterNode("Simplex Noise Filter", "Noise & Terrain", "PCGSimplexNoise",
                         "Multi-octave fractional Brownian motion noise sampler",
                         []() -> TERef<GraphNode> { return CreateRef<PCGSimplexNoiseNode>(); });
    palette.RegisterNode("Worley (Cellular) Noise", "Noise & Terrain", "PCGWorleyNoise",
                         "Cellular distance-based density noise",
                         []() -> TERef<GraphNode> { return CreateRef<PCGWorleyNoiseNode>(); });
    palette.RegisterNode("Elevation & Slope Filter", "Noise & Terrain", "PCGElevationSlope",
                         "Filters points by height range and surface slope angle",
                         []() -> TERef<GraphNode> { return CreateRef<PCGElevationSlopeNode>(); });

    // Structural
    palette.RegisterNode("Wilson's Maze Generator", "Structural", "PCGWilsonMaze",
                         "Uniform spanning tree algorithm for perfect maze corridors",
                         []() -> TERef<GraphNode> { return CreateRef<PCGWilsonMazeNode>(); });
    palette.RegisterNode("Cellular Automata (Caves)", "Structural", "PCGCellularAutomata",
                         "Organic cave and tunnel system generator",
                         []() -> TERef<GraphNode> { return CreateRef<PCGCellularAutomataNode>(); });
    palette.RegisterNode("BSP Room Carver", "Structural", "PCGBSPRoom",
                         "Binary Space Partitioning for rectangular dungeon rooms",
                         []() -> TERef<GraphNode> { return CreateRef<PCGBSPRoomNode>(); });
    palette.RegisterNode("Drunkard's Walk Cavern", "Structural", "PCGDrunkardWalk",
                         "Random walk cavern and winding tunnel generator",
                         []() -> TERef<GraphNode> { return CreateRef<PCGDrunkardWalkNode>(); });
    palette.RegisterNode("Wave Function Collapse (2D)", "Structural", "PCGWaveFunctionCollapse",
                         "Constraint-based tile pattern solver",
                         []() -> TERef<GraphNode> { return CreateRef<PCGWaveFunctionCollapseNode>(); });

    // 12 Specialized Processors
    palette.RegisterNode("Voronoi Partitioning", "Processors", "PCGVoronoiPartition",
                         "Generates biome and faction territory cells",
                         []() -> TERef<GraphNode> { return CreateRef<PCGVoronoiPartitionNode>(); });
    palette.RegisterNode("Delaunay Triangulation", "Processors", "PCGDelaunayTriangulation",
                         "Connects points into non-overlapping planar triangulations",
                         []() -> TERef<GraphNode> { return CreateRef<PCGDelaunayTriangulationNode>(); });
    palette.RegisterNode("Physics Point Relaxer (XPBD)", "Processors", "PCGPhysicsPointRelaxer",
                         "Soft spacing relaxation solver preventing crowding",
                         []() -> TERef<GraphNode> { return CreateRef<PCGPhysicsPointRelaxerNode>(); });
    palette.RegisterNode("Raycast Projection", "Processors", "PCGRaycastProjection",
                         "Snaps points to physics colliders or terrain surfaces",
                         []() -> TERef<GraphNode> { return CreateRef<PCGRaycastProjectionNode>(); });
    palette.RegisterNode("Bounds Exclusion Volume", "Processors", "PCGBoundsExclusion",
                         "Masks out spawn coordinates within exclusion zones",
                         []() -> TERef<GraphNode> { return CreateRef<PCGBoundsExclusionNode>(); });
    palette.RegisterNode("ECS Attribute Injector", "Processors", "PCGComponentAttributeInjector",
                         "Injects point attributes directly into ECS components",
                         []() -> TERef<GraphNode> { return CreateRef<PCGComponentAttributeInjectorNode>(); });
    palette.RegisterNode("Hierarchy Linker", "Processors", "PCGEntityHierarchyLinker",
                         "Creates parent-child relationships for spawned entities",
                         []() -> TERef<GraphNode> { return CreateRef<PCGEntityHierarchyLinkerNode>(); });
    palette.RegisterNode("L-System Fractal Generator", "Processors", "PCGLSystemGenerator",
                         "Grammar-driven branching generator for trees and rivers",
                         []() -> TERef<GraphNode> { return CreateRef<PCGLSystemGeneratorNode>(); });
    palette.RegisterNode("Edge / Border Extractor", "Processors", "PCGEdgeExtractor",
                         "Isolates perimeter points for walls and fences",
                         []() -> TERef<GraphNode> { return CreateRef<PCGEdgeExtractorNode>(); });
    palette.RegisterNode("Density Clustering (K-Means)", "Processors", "PCGKMeansClustering",
                         "Discovers point cluster centers for camps and POIs",
                         []() -> TERef<GraphNode> { return CreateRef<PCGKMeansClusteringNode>(); });
    palette.RegisterNode("A* Pathfinding Traversal", "Processors", "PCGAStarTraversal",
                         "Computes optimal road/path connectivity across point webs",
                         []() -> TERef<GraphNode> { return CreateRef<PCGAStarTraversalNode>(); });
    palette.RegisterNode("Proximity Attribute Mutator", "Processors", "PCGComponentProximityMutator",
                         "Modulates point attributes based on distance to targets",
                         []() -> TERef<GraphNode> { return CreateRef<PCGComponentProximityMutatorNode>(); });

    // Spawners
    palette.RegisterNode("Entity Spawner", "Spawners", "PCGEntitySpawner",
                         "Instantiates scene entities at computed point positions",
                         []() -> TERef<GraphNode> { return CreateRef<PCGEntitySpawnerNode>(); });
    palette.RegisterNode("Tilemap Spawner", "Spawners", "PCGTilemapSpawner", "Applies points to Tilemap layers",
                         []() -> TERef<GraphNode> { return CreateRef<PCGTilemapSpawnerNode>(); });
    palette.RegisterNode("Prototype Assembler", "Spawners", "PCGPrototypeAssembler",
                         "Spawns prototype assets at point locations",
                         []() -> TERef<GraphNode> { return CreateRef<PCGPrototypeAssemblerNode>(); });
}
