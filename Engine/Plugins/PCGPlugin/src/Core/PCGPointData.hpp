#pragma once

#include "Core/PreRequisites.h"
#include "Core/PCGPoint.hpp"
#include "GameFrameWork/GameplayUtils.hpp"

class PCGPointData
{
public:
    PCGPointData() = default;
    ~PCGPointData() = default;

    const TEArray<PCGPoint> &GetPoints() const { return m_Points; }
    TEArray<PCGPoint> &GetPoints() { return m_Points; }

    void AddPoint(const PCGPoint &point) { m_Points.Add(point); }
    void Clear() { m_Points.Clear(); }
    size_t GetCount() const { return m_Points.Size(); }
    bool IsEmpty() const { return m_Points.IsEmpty(); }

    void FilterByDensity(float minDensity, float maxDensity = 1.0f);
    void TransformPoints(const glm::vec3 &translation, const glm::quat &rotation, const glm::vec3 &scale);

    void UnionWith(const PCGPointData &other);
    void IntersectWith(const PCGPointData &other, float matchRadius = 1.0f);
    void DifferenceWith(const PCGPointData &other, float matchRadius = 1.0f);

    void GetBounds(glm::vec3 &outMin, glm::vec3 &outMax) const;

private:
    TEArray<PCGPoint> m_Points;
};

using PCGPointDataRef = TERef<PCGPointData>;
