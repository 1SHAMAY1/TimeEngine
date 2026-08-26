#include "Nodes/PCGSpawnerNodes.hpp"
#include "Core/Log.h"
#include "Core/Scene/BoxComponent.hpp"
#include "Core/Scene/CircleComponent.hpp"
#include "Core/Scene/EntityManager.hpp"
#include "Core/Scene/Scene.hpp"
#include "Core/Scene/TransformComponent.hpp"

// ==========================================
// PCGEntitySpawnerNode
// ==========================================
PCGEntitySpawnerNode::PCGEntitySpawnerNode()
{
    NodeType = "PCGEntitySpawner";
    Title = "Entity Spawner";
    Category = "Spawners";
    HeaderColor = TEColor(0.2f, 0.7f, 0.4f, 1.0f);
    Size = {220.0f, 140.0f};

    SetProperty("EntityPrefix", "PCG_Instance_");
    SetProperty("ComponentType", "BoxComponent"); // BoxComponent, CircleComponent, Empty

    AddInputPointPin("In");
    AddOutputPointPin("Out");
}

bool PCGEntitySpawnerNode::Execute(PCGExecutionContext &ctx)
{
    auto inData = GetInputPointData(ctx, "In");
    if (!inData)
        return false;

    TEString prefix = GetProperty("EntityPrefix", "PCG_Instance_");
    TEString compType = GetProperty("ComponentType", "BoxComponent");

    const auto &points = inData->GetPoints();

    if (ctx.TargetScene)
    {
        for (size_t i = 0; i < points.Size(); ++i)
        {
            const auto &pt = points[i];
            TEString entityName = prefix + TEString::FromInt(static_cast<int>(i));
            Entity e = ctx.TargetScene->CreateEntity(entityName.c_str());

            auto *tc = e.GetComponent<TransformComponent>();
            if (tc)
            {
                tc->Transform.Position = TEVector(pt.Position.x, pt.Position.y, pt.Position.z);
                tc->Transform.Scale = TEVector(pt.Scale.x, pt.Scale.y, pt.Scale.z);
            }

            if (compType == "BoxComponent")
            {
                auto *box = e.AddComponent<BoxComponent>();
                if (box)
                    box->BaseColor = TEColor(pt.Color.r, pt.Color.g, pt.Color.b, pt.Color.a);
            }
            else if (compType == "CircleComponent")
            {
                auto *circle = e.AddComponent<CircleComponent>();
                if (circle)
                    circle->BaseColor = TEColor(pt.Color.r, pt.Color.g, pt.Color.b, pt.Color.a);
            }
        }
    }

    SetOutputPointData(ctx, "Out", inData);
    return true;
}

// ==========================================
// PCGTilemapSpawnerNode
// ==========================================
PCGTilemapSpawnerNode::PCGTilemapSpawnerNode()
{
    NodeType = "PCGTilemapSpawner";
    Title = "Tilemap Spawner";
    Category = "Spawners";
    HeaderColor = TEColor(0.25f, 0.65f, 0.5f, 1.0f);
    Size = {220.0f, 130.0f};

    SetProperty("TileLayer", "Ground");
    SetProperty("TileSetAsset", "");

    AddInputPointPin("In");
    AddOutputPointPin("Out");
}

bool PCGTilemapSpawnerNode::Execute(PCGExecutionContext &ctx)
{
    auto inData = GetInputPointData(ctx, "In");
    if (!inData)
        return false;

    SetOutputPointData(ctx, "Out", inData);
    return true;
}

// ==========================================
// PCGPrototypeAssemblerNode
// ==========================================
PCGPrototypeAssemblerNode::PCGPrototypeAssemblerNode()
{
    NodeType = "PCGPrototypeAssembler";
    Title = "Prototype Assembler";
    Category = "Spawners";
    HeaderColor = TEColor(0.3f, 0.7f, 0.55f, 1.0f);
    Size = {220.0f, 130.0f};

    SetProperty("PrototypePath", "Assets/Prototypes/Tree.tePrototype");

    AddInputPointPin("In");
    AddOutputPointPin("Out");
}

bool PCGPrototypeAssemblerNode::Execute(PCGExecutionContext &ctx)
{
    auto inData = GetInputPointData(ctx, "In");
    if (!inData)
        return false;

    SetOutputPointData(ctx, "Out", inData);
    return true;
}
