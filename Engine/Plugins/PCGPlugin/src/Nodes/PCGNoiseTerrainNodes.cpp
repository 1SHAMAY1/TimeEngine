#include "Nodes/PCGNoiseTerrainNodes.hpp"
#include <cmath>

// Helper hash-based 2D noise generator
static float Hash2D(int x, int y, uint32_t seed)
{
    uint32_t h = seed + static_cast<uint32_t>(x) * 374761393u + static_cast<uint32_t>(y) * 668265263u;
    h = (h ^ (h >> 13)) * 1274126177u;
    return static_cast<float>(h ^ (h >> 16)) / 4294967295.0f;
}

static float Interpolate(float a, float b, float t)
{
    float ft = t * 3.1415927f;
    float f = (1.0f - std::cos(ft)) * 0.5f;
    return a * (1.0f - f) + b * f;
}

static float Perlin2D(float x, float y, uint32_t seed)
{
    int ix = static_cast<int>(std::floor(x));
    int iy = static_cast<int>(std::floor(y));
    float fx = x - static_cast<float>(ix);
    float fy = y - static_cast<float>(iy);

    float v1 = Hash2D(ix, iy, seed);
    float v2 = Hash2D(ix + 1, iy, seed);
    float v3 = Hash2D(ix, iy + 1, seed);
    float v4 = Hash2D(ix + 1, iy + 1, seed);

    float i1 = Interpolate(v1, v2, fx);
    float i2 = Interpolate(v3, v4, fx);
    return Interpolate(i1, i2, fy);
}

// ==========================================
// PCGPerlinNoiseNode
// ==========================================
PCGPerlinNoiseNode::PCGPerlinNoiseNode()
{
    NodeType = "PCGPerlinNoise";
    Title = "Perlin Noise Filter";
    Category = "Noise & Terrain";
    HeaderColor = TEColor(0.45f, 0.4f, 0.65f, 1.0f);
    Size = {210.0f, 140.0f};

    SetProperty("Scale", "0.05");
    SetProperty("Threshold", "0.4");
    SetProperty("ModulateDensity", "true");

    AddInputPointPin("In");
    AddOutputPointPin("Out");
}

bool PCGPerlinNoiseNode::Execute(PCGExecutionContext &ctx)
{
    auto inData = GetInputPointData(ctx, "In");
    if (!inData)
        return false;

    float scale = std::stof(GetProperty("Scale", "0.05").c_str());
    float threshold = std::stof(GetProperty("Threshold", "0.4").c_str());
    bool modDensity = GetProperty("ModulateDensity", "true") == "true";

    auto outData = CreateRef<PCGPointData>();
    const auto &points = inData->GetPoints();

    for (size_t i = 0; i < points.Size(); ++i)
    {
        PCGPoint pt = points[i];
        float n = Perlin2D(pt.Position.x * scale, pt.Position.y * scale, ctx.Seed);

        if (n >= threshold)
        {
            if (modDensity)
                pt.Density *= n;
            pt.SetAttribute("NoiseValue", PCGAttributeValue(n));
            outData->AddPoint(pt);
        }
    }

    SetOutputPointData(ctx, "Out", outData);
    return true;
}

// ==========================================
// PCGSimplexNoiseNode
// ==========================================
PCGSimplexNoiseNode::PCGSimplexNoiseNode()
{
    NodeType = "PCGSimplexNoise";
    Title = "Simplex Noise Filter";
    Category = "Noise & Terrain";
    HeaderColor = TEColor(0.5f, 0.45f, 0.7f, 1.0f);
    Size = {210.0f, 140.0f};

    SetProperty("Frequency", "0.08");
    SetProperty("Octaves", "3");

    AddInputPointPin("In");
    AddOutputPointPin("Out");
}

bool PCGSimplexNoiseNode::Execute(PCGExecutionContext &ctx)
{
    auto inData = GetInputPointData(ctx, "In");
    if (!inData)
        return false;

    float freq = std::stof(GetProperty("Frequency", "0.08").c_str());
    int octaves = std::max(1, std::stoi(GetProperty("Octaves", "3").c_str()));

    auto outData = CreateRef<PCGPointData>();
    const auto &points = inData->GetPoints();

    for (size_t i = 0; i < points.Size(); ++i)
    {
        PCGPoint pt = points[i];
        float total = 0.0f;
        float currentFreq = freq;
        float amplitude = 1.0f;
        float maxAmp = 0.0f;

        for (int oct = 0; oct < octaves; ++oct)
        {
            total += Perlin2D(pt.Position.x * currentFreq, pt.Position.y * currentFreq, ctx.Seed + oct * 31) * amplitude;
            maxAmp += amplitude;
            amplitude *= 0.5f;
            currentFreq *= 2.0f;
        }

        float normNoise = total / maxAmp;
        pt.Density *= normNoise;
        pt.SetAttribute("SimplexValue", PCGAttributeValue(normNoise));
        outData->AddPoint(pt);
    }

    SetOutputPointData(ctx, "Out", outData);
    return true;
}

// ==========================================
// PCGWorleyNoiseNode
// ==========================================
PCGWorleyNoiseNode::PCGWorleyNoiseNode()
{
    NodeType = "PCGWorleyNoise";
    Title = "Worley (Cellular) Noise";
    Category = "Noise & Terrain";
    HeaderColor = TEColor(0.4f, 0.5f, 0.7f, 1.0f);
    Size = {210.0f, 140.0f};

    SetProperty("CellSize", "15.0");
    SetProperty("Invert", "false");

    AddInputPointPin("In");
    AddOutputPointPin("Out");
}

bool PCGWorleyNoiseNode::Execute(PCGExecutionContext &ctx)
{
    auto inData = GetInputPointData(ctx, "In");
    if (!inData)
        return false;

    float cellSize = std::max(1.0f, std::stof(GetProperty("CellSize", "15.0").c_str()));
    bool invert = GetProperty("Invert", "false") == "true";

    auto outData = CreateRef<PCGPointData>();
    const auto &points = inData->GetPoints();

    for (size_t i = 0; i < points.Size(); ++i)
    {
        PCGPoint pt = points[i];
        int cellX = static_cast<int>(std::floor(pt.Position.x / cellSize));
        int cellY = static_cast<int>(std::floor(pt.Position.y / cellSize));

        float minDist = cellSize * 2.0f;

        for (int dx = -1; dx <= 1; ++dx)
        {
            for (int dy = -1; dy <= 1; ++dy)
            {
                int cx = cellX + dx;
                int cy = cellY + dy;
                float fx = static_cast<float>(cx) * cellSize + Hash2D(cx, cy, ctx.Seed) * cellSize;
                float fy = static_cast<float>(cy) * cellSize + Hash2D(cx, cy, ctx.Seed + 99) * cellSize;

                float dist = std::hypot(pt.Position.x - fx, pt.Position.y - fy);
                minDist = std::min(minDist, dist);
            }
        }

        float normVal = std::clamp(minDist / cellSize, 0.0f, 1.0f);
        if (invert)
            normVal = 1.0f - normVal;

        pt.Density *= normVal;
        pt.SetAttribute("CellularDistance", PCGAttributeValue(normVal));
        outData->AddPoint(pt);
    }

    SetOutputPointData(ctx, "Out", outData);
    return true;
}

// ==========================================
// PCGElevationSlopeNode
// ==========================================
PCGElevationSlopeNode::PCGElevationSlopeNode()
{
    NodeType = "PCGElevationSlope";
    Title = "Elevation & Slope Filter";
    Category = "Noise & Terrain";
    HeaderColor = TEColor(0.55f, 0.45f, 0.35f, 1.0f);
    Size = {220.0f, 140.0f};

    SetProperty("MinElevation", "-100.0");
    SetProperty("MaxElevation", "100.0");
    SetProperty("MaxSlopeAngleDeg", "45.0");

    AddInputPointPin("In");
    AddOutputPointPin("Out");
}

bool PCGElevationSlopeNode::Execute(PCGExecutionContext &ctx)
{
    auto inData = GetInputPointData(ctx, "In");
    if (!inData)
        return false;

    float minElev = std::stof(GetProperty("MinElevation", "-100.0").c_str());
    float maxElev = std::stof(GetProperty("MaxElevation", "100.0").c_str());
    float maxSlope = std::stof(GetProperty("MaxSlopeAngleDeg", "45.0").c_str());

    auto outData = CreateRef<PCGPointData>();
    const auto &points = inData->GetPoints();

    for (size_t i = 0; i < points.Size(); ++i)
    {
        const PCGPoint &pt = points[i];
        if (pt.Position.z >= minElev && pt.Position.z <= maxElev)
        {
            // Compute simulated slope from normal rotation
            glm::vec3 up = pt.Rotation * glm::vec3(0.0f, 1.0f, 0.0f);
            float slopeAngle = std::acos(std::clamp(glm::dot(up, glm::vec3(0.0f, 1.0f, 0.0f)), -1.0f, 1.0f)) * (180.0f / 3.1415927f);

            if (slopeAngle <= maxSlope)
            {
                outData->AddPoint(pt);
            }
        }
    }

    SetOutputPointData(ctx, "Out", outData);
    return true;
}
