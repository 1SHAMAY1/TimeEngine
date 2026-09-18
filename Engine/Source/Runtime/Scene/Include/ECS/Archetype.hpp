#pragma once
#include "PreRequisites.h"
#include "ECS/ComponentTypeID.hpp"
#include "ECS/ComponentMask.hpp"
#include <array>
#include <cstring>
#include <functional>
#include <type_traits>

using EntityID = uint64_t;
inline constexpr EntityID NullEntity = 0;

struct ComponentTypeInfo
{
    ComponentID ID = NullComponentID;
    size_t Size = 0;
    size_t Alignment = alignof(std::max_align_t);
    void (*ConstructFn)(void *dest) = nullptr;
    void (*CopyConstructFn)(void *dest, const void *src) = nullptr;
    void (*MoveConstructFn)(void *dest, void *src) = nullptr;
    void (*DestructFn)(void *dest) = nullptr;

    template <typename T>
    static ComponentTypeInfo Create()
    {
        using CleanT = std::remove_cvref_t<T>;
        ComponentTypeInfo info;
        info.ID = ComponentTypeID::Get<CleanT>();
        info.Size = sizeof(CleanT);
        info.Alignment = alignof(CleanT);

        info.ConstructFn = [](void *dest) {
            new (dest) CleanT();
        };

        info.CopyConstructFn = [](void *dest, const void *src) {
            new (dest) CleanT(*reinterpret_cast<const CleanT *>(src));
        };

        info.MoveConstructFn = [](void *dest, void *src) {
            new (dest) CleanT(std::move(*reinterpret_cast<CleanT *>(src)));
        };

        info.DestructFn = [](void *dest) {
            reinterpret_cast<CleanT *>(dest)->~CleanT();
        };

        return info;
    }
};

class ComponentColumn
{
public:
    ComponentColumn() = default;
    explicit ComponentColumn(const ComponentTypeInfo &info, size_t initialCapacity = 8)
        : m_Info(info), m_Capacity(initialCapacity), m_Size(0)
    {
        Allocate(m_Capacity);
    }

    ~ComponentColumn()
    {
        Clear();
    }

    ComponentColumn(const ComponentColumn &) = delete;
    ComponentColumn &operator=(const ComponentColumn &) = delete;

    ComponentColumn(ComponentColumn &&other) noexcept
        : m_Info(other.m_Info), m_Buffer(std::move(other.m_Buffer)), m_Capacity(other.m_Capacity), m_Size(other.m_Size)
    {
        other.m_Capacity = 0;
        other.m_Size = 0;
    }

    ComponentColumn &operator=(ComponentColumn &&other) noexcept
    {
        if (this != &other)
        {
            Clear();
            m_Info = other.m_Info;
            m_Buffer = std::move(other.m_Buffer);
            m_Capacity = other.m_Capacity;
            m_Size = other.m_Size;

            other.m_Capacity = 0;
            other.m_Size = 0;
        }
        return *this;
    }

    [[nodiscard]] void *GetRaw(size_t index)
    {
        return m_Buffer.GetData() + (index * m_Info.Size);
    }

    [[nodiscard]] const void *GetRaw(size_t index) const
    {
        return m_Buffer.GetData() + (index * m_Info.Size);
    }

    template <typename T>
    [[nodiscard]] T *Get(size_t index)
    {
        return reinterpret_cast<T *>(GetRaw(index));
    }

    template <typename T>
    [[nodiscard]] const T *Get(size_t index) const
    {
        return reinterpret_cast<const T *>(GetRaw(index));
    }

    template <typename T>
    [[nodiscard]] T *GetTypedData()
    {
        return reinterpret_cast<T *>(m_Buffer.GetData());
    }

    template <typename T>
    [[nodiscard]] const T *GetTypedData() const
    {
        return reinterpret_cast<const T *>(m_Buffer.GetData());
    }

    size_t PushDefault()
    {
        EnsureCapacity(m_Size + 1);
        void *dst = GetRaw(m_Size);
        if (m_Info.ConstructFn)
            m_Info.ConstructFn(dst);
        return m_Size++;
    }

    size_t PushCopy(const void *src)
    {
        EnsureCapacity(m_Size + 1);
        void *dst = GetRaw(m_Size);
        if (m_Info.CopyConstructFn)
            m_Info.CopyConstructFn(dst, src);
        return m_Size++;
    }

    size_t PushMove(void *src)
    {
        EnsureCapacity(m_Size + 1);
        void *dst = GetRaw(m_Size);
        if (m_Info.MoveConstructFn)
            m_Info.MoveConstructFn(dst, src);
        return m_Size++;
    }

    void RemoveSwapLast(size_t index)
    {
        if (index >= m_Size)
            return;

        size_t lastIdx = m_Size - 1;
        if (index != lastIdx)
        {
            void *target = GetRaw(index);
            void *last = GetRaw(lastIdx);
            if (m_Info.DestructFn)
                m_Info.DestructFn(target);
            if (m_Info.MoveConstructFn)
            {
                m_Info.MoveConstructFn(target, last);
                if (m_Info.DestructFn)
                    m_Info.DestructFn(last);
            }
        }
        else
        {
            void *target = GetRaw(index);
            if (m_Info.DestructFn)
                m_Info.DestructFn(target);
        }
        m_Size--;
    }

    void Clear()
    {
        if (m_Info.DestructFn)
        {
            for (size_t i = 0; i < m_Size; ++i)
            {
                m_Info.DestructFn(GetRaw(i));
            }
        }
        m_Size = 0;
    }

    [[nodiscard]] size_t Size() const { return m_Size; }
    [[nodiscard]] size_t Capacity() const { return m_Capacity; }
    [[nodiscard]] const ComponentTypeInfo &GetInfo() const { return m_Info; }

private:
    void Allocate(size_t capacity)
    {
        m_Capacity = capacity;
        m_Buffer.Resize(m_Capacity * m_Info.Size);
    }

    void EnsureCapacity(size_t needed)
    {
        if (needed <= m_Capacity)
            return;

        size_t newCap = m_Capacity == 0 ? 8 : (m_Capacity * 2);
        while (newCap < needed)
            newCap *= 2;

        TEArray<uint8_t> newBuffer;
        newBuffer.Resize(newCap * m_Info.Size);

        for (size_t i = 0; i < m_Size; ++i)
        {
            void *src = m_Buffer.GetData() + (i * m_Info.Size);
            void *dst = newBuffer.GetData() + (i * m_Info.Size);
            if (m_Info.MoveConstructFn)
            {
                m_Info.MoveConstructFn(dst, src);
                if (m_Info.DestructFn)
                    m_Info.DestructFn(src);
            }
        }

        m_Buffer = std::move(newBuffer);
        m_Capacity = newCap;
    }

    ComponentTypeInfo m_Info;
    TEArray<uint8_t> m_Buffer;
    size_t m_Capacity = 0;
    size_t m_Size = 0;
};

class TE_API Archetype
{
public:
    Archetype() = default;
    explicit Archetype(const ComponentMask &mask, const TEArray<ComponentTypeInfo> &types);
    ~Archetype() = default;

    Archetype(const Archetype &) = delete;
    Archetype &operator=(const Archetype &) = delete;
    Archetype(Archetype &&) noexcept = default;
    Archetype &operator=(Archetype &&) noexcept = default;

    [[nodiscard]] const ComponentMask &GetMask() const { return m_Mask; }
    [[nodiscard]] bool HasComponent(ComponentID id) const { return m_Mask.test(id); }
    [[nodiscard]] const TEArray<EntityID> &GetEntities() const { return m_Entities; }
    [[nodiscard]] size_t EntityCount() const { return m_Entities.Num(); }

    size_t AddEntity(EntityID id);
    EntityID RemoveEntitySwapLast(size_t row);

    template <typename T>
    [[nodiscard]] T *GetComponent(size_t row)
    {
        ComponentID id = ComponentTypeID::Get<T>();
        if (id < MaxComponentTypes && m_FastColumnMap[id] != static_cast<size_t>(-1))
        {
            return m_Columns[m_FastColumnMap[id]].Get<T>(row);
        }
        return nullptr;
    }

    template <typename T>
    [[nodiscard]] const T *GetComponent(size_t row) const
    {
        ComponentID id = ComponentTypeID::Get<T>();
        if (id < MaxComponentTypes && m_FastColumnMap[id] != static_cast<size_t>(-1))
        {
            return m_Columns[m_FastColumnMap[id]].Get<T>(row);
        }
        return nullptr;
    }

    [[nodiscard]] void *GetComponentRaw(ComponentID id, size_t row);
    [[nodiscard]] const void *GetComponentRaw(ComponentID id, size_t row) const;

    [[nodiscard]] ComponentColumn *GetColumn(ComponentID id);
    [[nodiscard]] const ComponentColumn *GetColumn(ComponentID id) const;

    const TEArray<ComponentTypeInfo> &GetTypeInfos() const { return m_TypeInfos; }

private:
    ComponentMask m_Mask;
    TEArray<ComponentTypeInfo> m_TypeInfos;
    TEArray<ComponentColumn> m_Columns;
    TEArray<EntityID> m_Entities;
    TEMap<ComponentID, size_t> m_ColumnIndexMap;
    std::array<size_t, MaxComponentTypes> m_FastColumnMap;
};
