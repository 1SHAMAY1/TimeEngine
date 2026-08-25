#include "Nodes/PCGStructuralNodes.hpp"
#include <algorithm>
#include <vector>

// ==========================================
// PCGWilsonMazeNode
// ==========================================
PCGWilsonMazeNode::PCGWilsonMazeNode()
{
    NodeType = "PCGWilsonMaze";
    Title = "Wilson's Maze Generator";
    Category = "Structural";
    HeaderColor = TEColor(0.6f, 0.35f, 0.2f, 1.0f);
    Size = {210.0f, 140.0f};

    SetProperty("GridWidth", "15");
    SetProperty("GridHeight", "15");
    SetProperty("CellSpacing", "2.0");

    AddOutputPointPin("Corridors");
    AddOutputPointPin("Walls");
}

bool PCGWilsonMazeNode::Execute(PCGExecutionContext &ctx)
{
    int w = std::clamp(std::stoi(GetProperty("GridWidth", "15").c_str()), 3, 100);
    int h = std::clamp(std::stoi(GetProperty("GridHeight", "15").c_str()), 3, 100);
    float spacing = std::max(0.5f, std::stof(GetProperty("CellSpacing", "2.0").c_str()));

    TEArray<bool> inTree(w * h, false);
    TEArray<int> next(w * h, -1);

    // Pick first root cell into tree
    inTree[0] = true;
    int unvisitedCount = w * h - 1;

    int directions[4][2] = {{0, -1}, {1, 0}, {0, 1}, {-1, 0}};

    while (unvisitedCount > 0)
    {
        // Pick random cell not in tree
        int u = ctx.RandomInt(0, w * h - 1);
        while (inTree[u])
        {
            u = (u + 1) % (w * h);
        }

        int curr = u;
        while (!inTree[curr])
        {
            int cx = curr % w;
            int cy = curr / w;

            int d = ctx.RandomInt(0, 3);
            int nx = cx + directions[d][0];
            int ny = cy + directions[d][1];

            if (nx >= 0 && nx < w && ny >= 0 && ny < h)
            {
                next[curr] = ny * w + nx;
                curr = next[curr];
            }
        }

        // Add loop-erased walk to tree
        curr = u;
        while (!inTree[curr])
        {
            inTree[curr] = true;
            unvisitedCount--;
            curr = next[curr];
        }
    }

    auto corridors = CreateRef<PCGPointData>();
    auto walls = CreateRef<PCGPointData>();

    float startX = ctx.Origin.x - (static_cast<float>(w) * spacing * 0.5f);
    float startY = ctx.Origin.y - (static_cast<float>(h) * spacing * 0.5f);

    for (int y = 0; y < h; ++y)
    {
        for (int x = 0; x < w; ++x)
        {
            int idx = y * w + x;
            glm::vec3 pos(startX + x * spacing, startY + y * spacing, 0.0f);
            if (inTree[idx])
            {
                PCGPoint pt(pos, 1.0f, spacing * 0.5f);
                pt.SetAttribute("CellType", PCGAttributeValue("Corridor"));
                corridors->AddPoint(pt);
            }
            else
            {
                PCGPoint pt(pos, 1.0f, spacing * 0.5f);
                pt.SetAttribute("CellType", PCGAttributeValue("Wall"));
                walls->AddPoint(pt);
            }
        }
    }

    SetOutputPointData(ctx, "Corridors", corridors);
    SetOutputPointData(ctx, "Walls", walls);
    return true;
}

// ==========================================
// PCGCellularAutomataNode
// ==========================================
PCGCellularAutomataNode::PCGCellularAutomataNode()
{
    NodeType = "PCGCellularAutomata";
    Title = "Cellular Automata (Caves)";
    Category = "Structural";
    HeaderColor = TEColor(0.55f, 0.35f, 0.25f, 1.0f);
    Size = {220.0f, 140.0f};

    SetProperty("Width", "25");
    SetProperty("Height", "25");
    SetProperty("FillPercent", "45");
    SetProperty("Iterations", "4");
    SetProperty("CellSize", "2.0");

    AddOutputPointPin("Floors");
    AddOutputPointPin("Walls");
}

bool PCGCellularAutomataNode::Execute(PCGExecutionContext &ctx)
{
    int w = std::clamp(std::stoi(GetProperty("Width", "25").c_str()), 5, 120);
    int h = std::clamp(std::stoi(GetProperty("Height", "25").c_str()), 5, 120);
    int fillPercent = std::clamp(std::stoi(GetProperty("FillPercent", "45").c_str()), 10, 90);
    int iterations = std::clamp(std::stoi(GetProperty("Iterations", "4").c_str()), 1, 10);
    float cellSize = std::max(0.5f, std::stof(GetProperty("CellSize", "2.0").c_str()));

    TEArray<int> map(w * h, 1);
    for (int y = 0; y < h; ++y)
    {
        for (int x = 0; x < w; ++x)
        {
            if (x == 0 || x == w - 1 || y == 0 || y == h - 1)
                map[y * w + x] = 1;
            else
                map[y * w + x] = (ctx.RandomInt(0, 100) < fillPercent) ? 1 : 0;
        }
    }

    for (int it = 0; it < iterations; ++it)
    {
        TEArray<int> nextMap = map;
        for (int y = 1; y < h - 1; ++y)
        {
            for (int x = 1; x < w - 1; ++x)
            {
                int wallCount = 0;
                for (int ny = y - 1; ny <= y + 1; ++ny)
                {
                    for (int nx = x - 1; nx <= x + 1; ++nx)
                    {
                        if (nx != x || ny != y)
                            wallCount += map[ny * w + nx];
                    }
                }
                nextMap[y * w + x] = (wallCount > 4) ? 1 : 0;
            }
        }
        map = nextMap;
    }

    auto floors = CreateRef<PCGPointData>();
    auto walls = CreateRef<PCGPointData>();

    float startX = ctx.Origin.x - (static_cast<float>(w) * cellSize * 0.5f);
    float startY = ctx.Origin.y - (static_cast<float>(h) * cellSize * 0.5f);

    for (int y = 0; y < h; ++y)
    {
        for (int x = 0; x < w; ++x)
        {
            glm::vec3 pos(startX + x * cellSize, startY + y * cellSize, 0.0f);
            if (map[y * w + x] == 0)
            {
                PCGPoint pt(pos, 1.0f, cellSize * 0.5f);
                pt.SetAttribute("TileType", PCGAttributeValue("Floor"));
                floors->AddPoint(pt);
            }
            else
            {
                PCGPoint pt(pos, 1.0f, cellSize * 0.5f);
                pt.SetAttribute("TileType", PCGAttributeValue("Wall"));
                walls->AddPoint(pt);
            }
        }
    }

    SetOutputPointData(ctx, "Floors", floors);
    SetOutputPointData(ctx, "Walls", walls);
    return true;
}

// ==========================================
// PCGBSPRoomNode
// ==========================================
PCGBSPRoomNode::PCGBSPRoomNode()
{
    NodeType = "PCGBSPRoom";
    Title = "BSP Room Carver";
    Category = "Structural";
    HeaderColor = TEColor(0.6f, 0.4f, 0.2f, 1.0f);
    Size = {210.0f, 140.0f};

    SetProperty("AreaWidth", "60.0");
    SetProperty("AreaHeight", "60.0");
    SetProperty("MinRoomSize", "8.0");
    SetProperty("Splits", "4");

    AddOutputPointPin("RoomPoints");
    AddOutputPointPin("CorridorPoints");
}

struct BSPRect
{
    float x, y, w, h;
    TEScope<BSPRect> left;
    TEScope<BSPRect> right;

    bool Split(float minSize, PCGExecutionContext &ctx)
    {
        if (left || right)
            return false;

        bool splitH = ctx.RandomFloat(0.0f, 1.0f) > 0.5f;
        if (w > h && w / h >= 1.25f)
            splitH = false;
        else if (h > w && h / w >= 1.25f)
            splitH = true;

        float max = (splitH ? h : w) - minSize;
        if (max <= minSize)
            return false;

        float split = ctx.RandomFloat(minSize, max);
        if (splitH)
        {
            left = CreateScope<BSPRect>(BSPRect{x, y, w, split});
            right = CreateScope<BSPRect>(BSPRect{x, y + split, w, h - split});
        }
        else
        {
            left = CreateScope<BSPRect>(BSPRect{x, y, split, h});
            right = CreateScope<BSPRect>(BSPRect{x + split, y, w - split, h});
        }
        return true;
    }
};

bool PCGBSPRoomNode::Execute(PCGExecutionContext &ctx)
{
    float areaW = std::stof(GetProperty("AreaWidth", "60.0").c_str());
    float areaH = std::stof(GetProperty("AreaHeight", "60.0").c_str());
    float minSize = std::max(4.0f, std::stof(GetProperty("MinRoomSize", "8.0").c_str()));
    int splits = std::clamp(std::stoi(GetProperty("Splits", "4").c_str()), 1, 8);

    BSPRect root{-areaW * 0.5f, -areaH * 0.5f, areaW, areaH};
    TEArray<BSPRect *> rects{&root};

    for (int s = 0; s < splits; ++s)
    {
        TEArray<BSPRect *> nextRects;
        for (auto *r : rects)
        {
            if (r->Split(minSize, ctx))
            {
                nextRects.push_back(r->left.get());
                nextRects.push_back(r->right.get());
            }
            else
            {
                nextRects.push_back(r);
            }
        }
        rects = nextRects;
    }

    auto roomData = CreateRef<PCGPointData>();
    for (auto *r : rects)
    {
        if (r)
        {
            glm::vec3 center(r->x + r->w * 0.5f, r->y + r->h * 0.5f, 0.0f);
            PCGPoint pt(center, 1.0f, std::min(r->w, r->h) * 0.5f);
            pt.SetAttribute("RoomWidth", PCGAttributeValue(r->w * 0.8f));
            pt.SetAttribute("RoomHeight", PCGAttributeValue(r->h * 0.8f));
            roomData->AddPoint(pt);
        }
    }

    SetOutputPointData(ctx, "RoomPoints", roomData);
    return true;
}

// ==========================================
// PCGDrunkardWalkNode
// ==========================================
PCGDrunkardWalkNode::PCGDrunkardWalkNode()
{
    NodeType = "PCGDrunkardWalk";
    Title = "Drunkard's Walk Cavern";
    Category = "Structural";
    HeaderColor = TEColor(0.55f, 0.45f, 0.2f, 1.0f);
    Size = {210.0f, 140.0f};

    SetProperty("Steps", "200");
    SetProperty("StepLength", "2.0");

    AddOutputPointPin("Out");
}

bool PCGDrunkardWalkNode::Execute(PCGExecutionContext &ctx)
{
    int steps = std::clamp(std::stoi(GetProperty("Steps", "200").c_str()), 10, 5000);
    float stepLen = std::max(0.5f, std::stof(GetProperty("StepLength", "2.0").c_str()));

    auto outData = CreateRef<PCGPointData>();
    glm::vec2 pos(ctx.Origin.x, ctx.Origin.y);

    glm::vec2 dirs[4] = {{0.0f, 1.0f}, {1.0f, 0.0f}, {0.0f, -1.0f}, {-1.0f, 0.0f}};

    for (int i = 0; i < steps; ++i)
    {
        outData->AddPoint(PCGPoint(glm::vec3(pos.x, pos.y, 0.0f), 1.0f, stepLen * 0.5f));
        int d = ctx.RandomInt(0, 3);
        pos += dirs[d] * stepLen;
    }

    SetOutputPointData(ctx, "Out", outData);
    return true;
}

// ==========================================
// PCGWaveFunctionCollapseNode
// ==========================================
PCGWaveFunctionCollapseNode::PCGWaveFunctionCollapseNode()
{
    NodeType = "PCGWaveFunctionCollapse";
    Title = "Wave Function Collapse (2D)";
    Category = "Structural";
    HeaderColor = TEColor(0.45f, 0.35f, 0.6f, 1.0f);
    Size = {230.0f, 140.0f};

    SetProperty("GridX", "16");
    SetProperty("GridY", "16");
    SetProperty("TileSize", "2.0");

    AddOutputPointPin("Tiles");
}

bool PCGWaveFunctionCollapseNode::Execute(PCGExecutionContext &ctx)
{
    int gx = std::clamp(std::stoi(GetProperty("GridX", "16").c_str()), 4, 64);
    int gy = std::clamp(std::stoi(GetProperty("GridY", "16").c_str()), 4, 64);
    float tileSize = std::max(0.5f, std::stof(GetProperty("TileSize", "2.0").c_str()));

    auto outData = CreateRef<PCGPointData>();
    float startX = ctx.Origin.x - (static_cast<float>(gx) * tileSize * 0.5f);
    float startY = ctx.Origin.y - (static_cast<float>(gy) * tileSize * 0.5f);

    const char *tileTypes[] = {"Grass", "Water", "Sand", "Forest", "Mountain"};
    int numTypes = 5;

    for (int y = 0; y < gy; ++y)
    {
        for (int x = 0; x < gx; ++x)
        {
            int choice = ctx.RandomInt(0, numTypes - 1);
            glm::vec3 pos(startX + x * tileSize, startY + y * tileSize, 0.0f);
            PCGPoint pt(pos, 1.0f, tileSize * 0.5f);
            pt.SetAttribute("TileTag", PCGAttributeValue(tileTypes[choice]));
            outData->AddPoint(pt);
        }
    }

    SetOutputPointData(ctx, "Tiles", outData);
    return true;
}
