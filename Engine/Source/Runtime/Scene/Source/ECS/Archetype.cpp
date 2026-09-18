#include "PreRequisites.h"
#include "ECS/Archetype.hpp"

Archetype::Archetype(const ComponentMask &mask, const TEArray<ComponentTypeInfo> &types)
    : m_Mask(mask), m_TypeInfos(types)
{
    m_FastColumnMap.fill(static_cast<size_t>(-1));
    for (size_t i = 0; i < m_TypeInfos.Num(); ++i)
    {
        const auto &info = m_TypeInfos[i];
        m_Columns.Add(ComponentColumn(info));
        m_ColumnIndexMap[info.ID] = i;
        if (info.ID < MaxComponentTypes)
        {
            m_FastColumnMap[info.ID] = i;
        }
    }
}

size_t Archetype::AddEntity(EntityID id)
{
    size_t row = m_Entities.Num();
    m_Entities.Add(id);
    for (auto &col : m_Columns)
    {
        col.PushDefault();
    }
    return row;
}

EntityID Archetype::RemoveEntitySwapLast(size_t row)
{
    if (row >= m_Entities.Num())
        return NullEntity;

    size_t lastRow = m_Entities.Num() - 1;
    EntityID movedEntity = NullEntity;

    if (row != lastRow)
    {
        movedEntity = m_Entities[lastRow];
        m_Entities[row] = movedEntity;
    }

    m_Entities.RemoveAt(lastRow);

    for (auto &col : m_Columns)
    {
        col.RemoveSwapLast(row);
    }

    return movedEntity;
}

void *Archetype::GetComponentRaw(ComponentID id, size_t row)
{
    if (id < MaxComponentTypes && m_FastColumnMap[id] != static_cast<size_t>(-1))
    {
        return m_Columns[m_FastColumnMap[id]].GetRaw(row);
    }
    return nullptr;
}

const void *Archetype::GetComponentRaw(ComponentID id, size_t row) const
{
    if (id < MaxComponentTypes && m_FastColumnMap[id] != static_cast<size_t>(-1))
    {
        return m_Columns[m_FastColumnMap[id]].GetRaw(row);
    }
    return nullptr;
}

ComponentColumn *Archetype::GetColumn(ComponentID id)
{
    if (id < MaxComponentTypes && m_FastColumnMap[id] != static_cast<size_t>(-1))
    {
        return &m_Columns[m_FastColumnMap[id]];
    }
    return nullptr;
}

const ComponentColumn *Archetype::GetColumn(ComponentID id) const
{
    if (id < MaxComponentTypes && m_FastColumnMap[id] != static_cast<size_t>(-1))
    {
        return &m_Columns[m_FastColumnMap[id]];
    }
    return nullptr;
}

