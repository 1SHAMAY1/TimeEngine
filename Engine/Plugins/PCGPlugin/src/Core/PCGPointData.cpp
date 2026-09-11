#include "Core/PCGPointData.hpp"
#include "Utils/MathUtils.hpp"
#include <algorithm>

void PCGPointData::FilterByDensity(float minDensity, float maxDensity)
{
    TEArray<PCGPoint> filtered;
    for (size_t i = 0; i < m_Points.Size(); ++i)
    {
        if (m_Points[i].Density >= minDensity && m_Points[i].Density <= maxDensity)
        {
            filtered.Add(m_Points[i]);
        }
    }
    m_Points = filtered;
}

void PCGPointData::TransformPoints(const TEVector &translation, const TEQuat &rotation, const TEVector &scale)
{
    for (size_t i = 0; i < m_Points.Size(); ++i)
    {
        TEVector scaledPos(m_Points[i].Position.x * scale.x, m_Points[i].Position.y * scale.y,
                           m_Points[i].Position.z * scale.z);
        m_Points[i].Position = translation + (rotation * scaledPos);
        m_Points[i].Rotation = rotation;
        m_Points[i].Scale.x *= scale.x;
        m_Points[i].Scale.y *= scale.y;
        m_Points[i].Scale.z *= scale.z;
    }
}

void PCGPointData::UnionWith(const PCGPointData &other)
{
    const auto &otherPoints = other.GetPoints();
    for (size_t i = 0; i < otherPoints.Size(); ++i)
    {
        m_Points.Add(otherPoints[i]);
    }
}

void PCGPointData::IntersectWith(const PCGPointData &other, float matchRadius)
{
    const auto &otherPoints = other.GetPoints();
    float rSq = matchRadius * matchRadius;

    TEArray<PCGPoint> result;
    for (size_t i = 0; i < m_Points.Size(); ++i)
    {
        bool foundMatch = false;
        for (size_t j = 0; j < otherPoints.Size(); ++j)
        {
            TEVector diff = m_Points[i].Position - otherPoints[j].Position;
            if ((diff.x * diff.x + diff.y * diff.y + diff.z * diff.z) <= rSq)
            {
                foundMatch = true;
                break;
            }
        }
        if (foundMatch)
        {
            result.Add(m_Points[i]);
        }
    }
    m_Points = result;
}

void PCGPointData::DifferenceWith(const PCGPointData &other, float matchRadius)
{
    const auto &otherPoints = other.GetPoints();
    float rSq = matchRadius * matchRadius;

    TEArray<PCGPoint> result;
    for (size_t i = 0; i < m_Points.Size(); ++i)
    {
        bool foundMatch = false;
        for (size_t j = 0; j < otherPoints.Size(); ++j)
        {
            TEVector diff = m_Points[i].Position - otherPoints[j].Position;
            if ((diff.x * diff.x + diff.y * diff.y + diff.z * diff.z) <= rSq)
            {
                foundMatch = true;
                break;
            }
        }
        if (!foundMatch)
        {
            result.Add(m_Points[i]);
        }
    }
    m_Points = result;
}

void PCGPointData::GetBounds(TEVector &outMin, TEVector &outMax) const
{
    if (m_Points.IsEmpty())
    {
        outMin = TEVector(0.0f, 0.0f, 0.0f);
        outMax = TEVector(0.0f, 0.0f, 0.0f);
        return;
    }

    outMin = m_Points[0].Position;
    outMax = m_Points[0].Position;

    for (size_t i = 1; i < m_Points.Size(); ++i)
    {
        outMin.x = std::min(outMin.x, m_Points[i].Position.x);
        outMin.y = std::min(outMin.y, m_Points[i].Position.y);
        outMin.z = std::min(outMin.z, m_Points[i].Position.z);

        outMax.x = std::max(outMax.x, m_Points[i].Position.x);
        outMax.y = std::max(outMax.y, m_Points[i].Position.y);
        outMax.z = std::max(outMax.z, m_Points[i].Position.z);
    }
}
