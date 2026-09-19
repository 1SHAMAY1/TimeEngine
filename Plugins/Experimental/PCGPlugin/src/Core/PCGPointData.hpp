#pragma once

#include "PCGPoint.hpp"
#include "PreRequisites.h"
#include "GameplayUtils.hpp"

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
    void TransformPoints(const TEVector &translation, const TEQuat &rotation, const TEVector &scale);

    void UnionWith(const PCGPointData &other);
    void IntersectWith(const PCGPointData &other, float matchRadius = 1.0f);
    void DifferenceWith(const PCGPointData &other, float matchRadius = 1.0f);

    void GetBounds(TEVector &outMin, TEVector &outMax) const;

private:
    TEArray<PCGPoint> m_Points;
};

using PCGPointDataRef = TERef<PCGPointData>;
