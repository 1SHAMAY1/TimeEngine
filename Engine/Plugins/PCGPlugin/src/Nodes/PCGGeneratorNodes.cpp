#include "Nodes/PCGGeneratorNodes.hpp"
#include <cmath>
#include <vector>

// ==========================================
// PCGGridGeneratorNode
// ==========================================
PCGGridGeneratorNode::PCGGridGeneratorNode()
{
    NodeType = "PCGGridGenerator";
    Title = "Grid Generator";
    Category = "Generators";
    HeaderColor = TEColor(0.2f, 0.6f, 0.4f, 1.0f);
    Size = {200.0f, 130.0f};

    SetProperty("CellSizeX", "2.0");
    SetProperty("CellSizeY", "2.0");
    SetProperty("Jitter", "0.0");

    AddOutputPointPin("Out");
}

bool PCGGridGeneratorNode::Execute(PCGExecutionContext &ctx)
{
    float cellX = std::max(0.1f, std::stof(GetProperty("CellSizeX", "2.0").c_str()));
    float cellY = std::max(0.1f, std::stof(GetProperty("CellSizeY", "2.0").c_str()));
    float jitter = std::stof(GetProperty("Jitter", "0.0").c_str());

    auto outData = CreateRef<PCGPointData>();

    for (float x = ctx.BoundsMin.x; x <= ctx.BoundsMax.x; x += cellX)
    {
        for (float y = ctx.BoundsMin.y; y <= ctx.BoundsMax.y; y += cellY)
        {
            float jx = (jitter > 0.0f) ? ctx.RandomFloat(-jitter, jitter) : 0.0f;
            float jy = (jitter > 0.0f) ? ctx.RandomFloat(-jitter, jitter) : 0.0f;

            PCGPoint pt(glm::vec3(x + jx, y + jy, 0.0f), 1.0f, std::min(cellX, cellY) * 0.5f);
            pt.Seed = ctx.Seed + static_cast<uint32_t>(outData->GetCount());
            outData->AddPoint(pt);
        }
    }

    SetOutputPointData(ctx, "Out", outData);
    return true;
}

// ==========================================
// PCGRandomScatterNode
// ==========================================
PCGRandomScatterNode::PCGRandomScatterNode()
{
    NodeType = "PCGRandomScatter";
    Title = "Random Scatter";
    Category = "Generators";
    HeaderColor = TEColor(0.25f, 0.65f, 0.45f, 1.0f);
    Size = {200.0f, 130.0f};

    SetProperty("PointCount", "100");
    SetProperty("RandomRotation", "true");

    AddOutputPointPin("Out");
}

bool PCGRandomScatterNode::Execute(PCGExecutionContext &ctx)
{
    int count = std::max(1, std::stoi(GetProperty("PointCount", "100").c_str()));
    bool randomRot = GetProperty("RandomRotation", "true") == "true";

    auto outData = CreateRef<PCGPointData>();

    for (int i = 0; i < count; ++i)
    {
        float rx = ctx.RandomFloat(ctx.BoundsMin.x, ctx.BoundsMax.x);
        float ry = ctx.RandomFloat(ctx.BoundsMin.y, ctx.BoundsMax.y);

        PCGPoint pt(glm::vec3(rx, ry, 0.0f), 1.0f, 0.5f);
        if (randomRot)
        {
            float angle = ctx.RandomFloat(0.0f, 6.2831853f);
            pt.Rotation = glm::angleAxis(angle, glm::vec3(0.0f, 0.0f, 1.0f));
        }
        pt.Seed = ctx.Seed + static_cast<uint32_t>(i);
        outData->AddPoint(pt);
    }

    SetOutputPointData(ctx, "Out", outData);
    return true;
}

// ==========================================
// PCGPoissonDiskNode
// ==========================================
PCGPoissonDiskNode::PCGPoissonDiskNode()
{
    NodeType = "PCGPoissonDisk";
    Title = "Poisson Disk Sampler";
    Category = "Generators";
    HeaderColor = TEColor(0.2f, 0.55f, 0.6f, 1.0f);
    Size = {220.0f, 130.0f};

    SetProperty("MinDistance", "3.0");
    SetProperty("MaxCandidates", "30");

    AddOutputPointPin("Out");
}

bool PCGPoissonDiskNode::Execute(PCGExecutionContext &ctx)
{
    float r = std::max(0.5f, std::stof(GetProperty("MinDistance", "3.0").c_str()));
    int k = std::max(5, std::stoi(GetProperty("MaxCandidates", "30").c_str()));

    float cellSize = r / std::sqrt(2.0f);
    int gridW = std::max(1, static_cast<int>(std::ceil((ctx.BoundsMax.x - ctx.BoundsMin.x) / cellSize)));
    int gridH = std::max(1, static_cast<int>(std::ceil((ctx.BoundsMax.y - ctx.BoundsMin.y) / cellSize)));

    TEArray<int> grid(gridW * gridH, -1);
    TEArray<glm::vec2> processList;
    auto outData = CreateRef<PCGPointData>();

    auto toGrid = [&](const glm::vec2 &p) -> glm::ivec2 {
        return glm::ivec2(
            std::clamp(static_cast<int>((p.x - ctx.BoundsMin.x) / cellSize), 0, gridW - 1),
            std::clamp(static_cast<int>((p.y - ctx.BoundsMin.y) / cellSize), 0, gridH - 1)
        );
    };

    glm::vec2 firstPoint(
        ctx.RandomFloat(ctx.BoundsMin.x, ctx.BoundsMax.x),
        ctx.RandomFloat(ctx.BoundsMin.y, ctx.BoundsMax.y)
    );
    processList.push_back(firstPoint);
    outData->AddPoint(PCGPoint(glm::vec3(firstPoint.x, firstPoint.y, 0.0f), 1.0f, r * 0.5f));
    glm::ivec2 g0 = toGrid(firstPoint);
    grid[g0.y * gridW + g0.x] = 0;

    while (!processList.empty())
    {
        int idx = ctx.RandomInt(0, static_cast<int>(processList.size()) - 1);
        glm::vec2 current = processList[idx];
        bool found = false;

        for (int step = 0; step < k; ++step)
        {
            float angle = ctx.RandomFloat(0.0f, 6.2831853f);
            float dist = ctx.RandomFloat(r, 2.0f * r);
            glm::vec2 candidate = current + glm::vec2(std::cos(angle) * dist, std::sin(angle) * dist);

            if (candidate.x < ctx.BoundsMin.x || candidate.x > ctx.BoundsMax.x ||
                candidate.y < ctx.BoundsMin.y || candidate.y > ctx.BoundsMax.y)
                continue;

            glm::ivec2 cg = toGrid(candidate);
            bool ok = true;

            int minX = std::max(0, cg.x - 2);
            int maxX = std::min(gridW - 1, cg.x + 2);
            int minY = std::max(0, cg.y - 2);
            int maxY = std::min(gridH - 1, cg.y + 2);

            for (int gx = minX; gx <= maxX && ok; ++gx)
            {
                for (int gy = minY; gy <= maxY && ok; ++gy)
                {
                    int ptIdx = grid[gy * gridW + gx];
                    if (ptIdx != -1)
                    {
                        const auto &neighbor = outData->GetPoints()[ptIdx];
                        float d = glm::distance(candidate, glm::vec2(neighbor.Position.x, neighbor.Position.y));
                        if (d < r)
                            ok = false;
                    }
                }
            }

            if (ok)
            {
                found = true;
                int newPtIdx = static_cast<int>(outData->GetCount());
                outData->AddPoint(PCGPoint(glm::vec3(candidate.x, candidate.y, 0.0f), 1.0f, r * 0.5f));
                processList.push_back(candidate);
                grid[cg.y * gridW + cg.x] = newPtIdx;
                break;
            }
        }

        if (!found)
        {
            processList.erase(processList.begin() + idx);
        }
    }

    SetOutputPointData(ctx, "Out", outData);
    return true;
}

// ==========================================
// PCGSplinePathNode
// ==========================================
PCGSplinePathNode::PCGSplinePathNode()
{
    NodeType = "PCGSplinePath";
    Title = "Spline / Path Sampler";
    Category = "Generators";
    HeaderColor = TEColor(0.35f, 0.55f, 0.4f, 1.0f);
    Size = {210.0f, 130.0f};

    SetProperty("StepDistance", "1.5");
    SetProperty("StartX", "-40.0");
    SetProperty("StartY", "0.0");
    SetProperty("EndX", "40.0");
    SetProperty("EndY", "0.0");
    SetProperty("Curvature", "10.0");

    AddOutputPointPin("Out");
}

bool PCGSplinePathNode::Execute(PCGExecutionContext &ctx)
{
    float step = std::max(0.1f, std::stof(GetProperty("StepDistance", "1.5").c_str()));
    float sx = std::stof(GetProperty("StartX", "-40.0").c_str());
    float sy = std::stof(GetProperty("StartY", "0.0").c_str());
    float ex = std::stof(GetProperty("EndX", "40.0").c_str());
    float ey = std::stof(GetProperty("EndY", "0.0").c_str());
    float curve = std::stof(GetProperty("Curvature", "10.0").c_str());

    glm::vec2 p0(sx, sy);
    glm::vec2 p2(ex, ey);
    glm::vec2 mid = (p0 + p2) * 0.5f;
    glm::vec2 dir = glm::normalize(p2 - p0);
    glm::vec2 perp(-dir.y, dir.x);
    glm::vec2 p1 = mid + perp * curve;

    auto outData = CreateRef<PCGPointData>();
    float totalDist = glm::distance(p0, p2);
    int numSteps = std::max(2, static_cast<int>(totalDist / step));

    for (int i = 0; i <= numSteps; ++i)
    {
        float t = static_cast<float>(i) / static_cast<float>(numSteps);
        // Quadratic bezier
        glm::vec2 pos = (1.0f - t) * (1.0f - t) * p0 + 2.0f * (1.0f - t) * t * p1 + t * t * p2;
        glm::vec2 tangent = glm::normalize(2.0f * (1.0f - t) * (p1 - p0) + 2.0f * t * (p2 - p1));

        float angle = std::atan2(tangent.y, tangent.x);
        PCGPoint pt(glm::vec3(pos.x, pos.y, 0.0f), 1.0f, step * 0.5f);
        pt.Rotation = glm::angleAxis(angle, glm::vec3(0.0f, 0.0f, 1.0f));
        outData->AddPoint(pt);
    }

    SetOutputPointData(ctx, "Out", outData);
    return true;
}
