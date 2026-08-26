#include "Core/PCGPointData.hpp"
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

void PCGPointData::TransformPoints(const glm::vec3 &translation, const glm::quat &rotation, const glm::vec3 &scale)
{
    for (size_t i = 0; i < m_Points.Size(); ++i)
    {
        m_Points[i].Position = translation + (rotation * (m_Points[i].Position * scale));
        m_Points[i].Rotation = rotation * m_Points[i].Rotation;
        m_Points[i].Scale *= scale;
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
            glm::vec3 diff = m_Points[i].Position - otherPoints[j].Position;
            if (glm::dot(diff, diff) <= rSq)
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
            glm::vec3 diff = m_Points[i].Position - otherPoints[j].Position;
            if (glm::dot(diff, diff) <= rSq)
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

void PCGPointData::GetBounds(glm::vec3 &outMin, glm::vec3 &outMax) const
{
    if (m_Points.IsEmpty())
    {
        outMin = glm::vec3(0.0f);
        outMax = glm::vec3(0.0f);
        return;
    }

    outMin = m_Points[0].Position;
    outMax = m_Points[0].Position;

    for (size_t i = 1; i < m_Points.Size(); ++i)
    {
        outMin = glm::min(outMin, m_Points[i].Position);
        outMax = glm::max(outMax, m_Points[i].Position);
    }
}
