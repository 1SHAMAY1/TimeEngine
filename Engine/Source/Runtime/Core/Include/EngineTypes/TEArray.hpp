#pragma once
#include "EngineTypes/TEOption.hpp"
#include "EngineTypes/TEResult.hpp"
#include "EngineTypes/TESpan.hpp"
#include <algorithm>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <initializer_list>
#include <iterator>
#include <new>
#include <stdexcept>
#include <type_traits>
#include <utility>

class TEString;
struct TEVector;
struct TEVector2;
struct TEVector4;

// ==========================================
// TEArray - Dynamic Contiguous Engine Container Template
// ==========================================
template <typename T> class TEArray
{
public:
    using value_type = T;
    using size_type = size_t;
    using difference_type = ptrdiff_t;
    using reference = T &;
    using const_reference = const T &;
    using pointer = T *;
    using const_pointer = const T *;
    using iterator = T *;
    using const_iterator = const T *;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

private:
    T *m_Data = nullptr;
    size_t m_Size = 0;
    size_t m_Capacity = 0;

    void Reallocate(size_t newCapacity)
    {
        if (newCapacity == m_Capacity)
            return;

        if (newCapacity == 0)
        {
            DestroyRange(m_Data, m_Size);
            if (m_Data)
                std::free(m_Data);
            m_Data = nullptr;
            m_Size = 0;
            m_Capacity = 0;
            return;
        }

        T *newData = static_cast<T *>(std::malloc(newCapacity * sizeof(T)));
        if (!newData)
            return;

        size_t moveCount = (std::min)(m_Size, newCapacity);
        if (m_Data)
        {
            if constexpr (std::is_trivially_copyable_v<T>)
            {
                if (moveCount > 0)
                    std::memcpy(newData, m_Data, moveCount * sizeof(T));
            }
            else
            {
                for (size_t i = 0; i < moveCount; ++i)
                {
                    new (&newData[i]) T(std::move(m_Data[i]));
                    m_Data[i].~T();
                }
            }
            if (m_Size > newCapacity)
            {
                for (size_t i = newCapacity; i < m_Size; ++i)
                {
                    m_Data[i].~T();
                }
            }
            std::free(m_Data);
        }

        m_Data = newData;
        m_Size = moveCount;
        m_Capacity = newCapacity;
    }

    void DestroyRange(T *data, size_t count)
    {
        if (!data || count == 0)
            return;
        if constexpr (!std::is_trivially_destructible_v<T>)
        {
            for (size_t i = 0; i < count; ++i)
            {
                data[i].~T();
            }
        }
    }

    size_t GrowthCapacity(size_t targetCapacity) const
    {
        size_t growth = m_Capacity + (m_Capacity >> 1);
        if (growth < targetCapacity)
            growth = targetCapacity;
        return (std::max)(growth, static_cast<size_t>(4));
    }

public:
    TEArray() noexcept = default;

    TEArray(std::initializer_list<T> initList)
    {
        Reserve(initList.size());
        for (const auto &item : initList)
        {
            Add(item);
        }
    }

    explicit TEArray(size_t count)
    {
        Resize(count);
    }

    TEArray(size_t count, const T &value)
    {
        Reserve(count);
        for (size_t i = 0; i < count; ++i)
        {
            Add(value);
        }
    }

    template <typename InputIt, typename = std::enable_if_t<!std::is_integral_v<InputIt>>>
    TEArray(InputIt first, InputIt last)
    {
        Assign(first, last);
    }

    ~TEArray()
    {
        DestroyRange(m_Data, m_Size);
        if (m_Data)
        {
            std::free(m_Data);
            m_Data = nullptr;
        }
        m_Size = 0;
        m_Capacity = 0;
    }

    TEArray(const TEArray &other)
    {
        if (other.m_Size > 0)
        {
            m_Data = static_cast<T *>(std::malloc(other.m_Size * sizeof(T)));
            if (m_Data)
            {
                m_Capacity = other.m_Size;
                m_Size = other.m_Size;
                if constexpr (std::is_trivially_copyable_v<T>)
                {
                    std::memcpy(m_Data, other.m_Data, other.m_Size * sizeof(T));
                }
                else
                {
                    for (size_t i = 0; i < other.m_Size; ++i)
                    {
                        new (&m_Data[i]) T(other.m_Data[i]);
                    }
                }
            }
        }
    }

    TEArray(TEArray &&other) noexcept
        : m_Data(other.m_Data), m_Size(other.m_Size), m_Capacity(other.m_Capacity)
    {
        other.m_Data = nullptr;
        other.m_Size = 0;
        other.m_Capacity = 0;
    }

    TEArray &operator=(const TEArray &other)
    {
        if (this != &other)
        {
            Clear();
            Reserve(other.m_Size);
            for (size_t i = 0; i < other.m_Size; ++i)
            {
                Add(other.m_Data[i]);
            }
        }
        return *this;
    }

    TEArray &operator=(TEArray &&other) noexcept
    {
        if (this != &other)
        {
            DestroyRange(m_Data, m_Size);
            if (m_Data)
                std::free(m_Data);

            m_Data = other.m_Data;
            m_Size = other.m_Size;
            m_Capacity = other.m_Capacity;

            other.m_Data = nullptr;
            other.m_Size = 0;
            other.m_Capacity = 0;
        }
        return *this;
    }

    TEArray &operator=(std::initializer_list<T> initList)
    {
        Assign(initList);
        return *this;
    }

    // Capacity & Size
    [[nodiscard]] size_t Num() const noexcept { return m_Size; }
    [[nodiscard]] size_t Size() const noexcept { return m_Size; }
    [[nodiscard]] size_t size() const noexcept { return m_Size; }
    [[nodiscard]] size_t Capacity() const noexcept { return m_Capacity; }
    [[nodiscard]] size_t capacity() const noexcept { return m_Capacity; }
    [[nodiscard]] bool IsEmpty() const noexcept { return m_Size == 0; }
    [[nodiscard]] bool empty() const noexcept { return m_Size == 0; }

    void Reserve(size_t newCapacity)
    {
        if (newCapacity > m_Capacity)
        {
            Reallocate(newCapacity);
        }
    }
    void reserve(size_t newCapacity) { Reserve(newCapacity); }

    void Resize(size_t newSize, const T &value = T())
    {
        if (newSize > m_Size)
        {
            if (newSize > m_Capacity)
                Reserve(GrowthCapacity(newSize));

            for (size_t i = m_Size; i < newSize; ++i)
            {
                new (&m_Data[i]) T(value);
            }
            m_Size = newSize;
        }
        else if (newSize < m_Size)
        {
            for (size_t i = newSize; i < m_Size; ++i)
            {
                m_Data[i].~T();
            }
            m_Size = newSize;
        }
    }
    void resize(size_t newSize, const T &value = T()) { Resize(newSize, value); }

    void Shrink()
    {
        if (m_Capacity > m_Size)
        {
            Reallocate(m_Size);
        }
    }
    void shrink_to_fit() { Shrink(); }

    // Assignment & Modification
    void Assign(size_t count, const T &value)
    {
        Clear();
        Reserve(count);
        for (size_t i = 0; i < count; ++i)
        {
            Add(value);
        }
    }

    template <typename InputIt, typename = std::enable_if_t<!std::is_integral_v<InputIt>>>
    void Assign(InputIt first, InputIt last)
    {
        Clear();
        for (auto it = first; it != last; ++it)
        {
            Add(*it);
        }
    }

    void Assign(std::initializer_list<T> ilist)
    {
        Clear();
        Reserve(ilist.size());
        for (const auto &item : ilist)
        {
            Add(item);
        }
    }

    void assign(size_t count, const T &value) { Assign(count, value); }
    template <typename InputIt, typename = std::enable_if_t<!std::is_integral_v<InputIt>>>
    void assign(InputIt first, InputIt last) { Assign(first, last); }
    void assign(std::initializer_list<T> ilist) { Assign(ilist); }

    // Add & Insert
    void Add(const T &element)
    {
        if (m_Size >= m_Capacity)
            Reserve(GrowthCapacity(m_Size + 1));

        new (&m_Data[m_Size]) T(element);
        ++m_Size;
    }

    void Add(T &&element)
    {
        if (m_Size >= m_Capacity)
            Reserve(GrowthCapacity(m_Size + 1));

        new (&m_Data[m_Size]) T(std::move(element));
        ++m_Size;
    }

    void push_back(const T &element) { Add(element); }
    void push_back(T &&element) { Add(std::move(element)); }

    void pop_back()
    {
        if (m_Size > 0)
        {
            --m_Size;
            m_Data[m_Size].~T();
        }
    }

    template <typename... Args> reference Emplace(Args &&...args)
    {
        if (m_Size >= m_Capacity)
            Reserve(GrowthCapacity(m_Size + 1));

        new (&m_Data[m_Size]) T(std::forward<Args>(args)...);
        reference ref = m_Data[m_Size];
        ++m_Size;
        return ref;
    }

    template <typename... Args> reference emplace_back(Args &&...args)
    {
        return Emplace(std::forward<Args>(args)...);
    }

    void Insert(size_t index, const T &element)
    {
        if (index > m_Size)
            return;

        if (m_Size >= m_Capacity)
            Reserve(GrowthCapacity(m_Size + 1));

        if (index < m_Size)
        {
            new (&m_Data[m_Size]) T(std::move(m_Data[m_Size - 1]));
            for (size_t i = m_Size - 1; i > index; --i)
            {
                m_Data[i] = std::move(m_Data[i - 1]);
            }
            m_Data[index] = element;
        }
        else
        {
            new (&m_Data[m_Size]) T(element);
        }
        ++m_Size;
    }

    iterator insert(const_iterator pos, const T &value)
    {
        size_t index = static_cast<size_t>(pos - m_Data);
        Insert(index, value);
        return m_Data + index;
    }

    iterator insert(const_iterator pos, T &&value)
    {
        size_t index = static_cast<size_t>(pos - m_Data);
        if (index > m_Size)
            index = m_Size;

        if (m_Size >= m_Capacity)
            Reserve(GrowthCapacity(m_Size + 1));

        if (index < m_Size)
        {
            new (&m_Data[m_Size]) T(std::move(m_Data[m_Size - 1]));
            for (size_t i = m_Size - 1; i > index; --i)
            {
                m_Data[i] = std::move(m_Data[i - 1]);
            }
            m_Data[index] = std::move(value);
        }
        else
        {
            new (&m_Data[m_Size]) T(std::move(value));
        }
        ++m_Size;
        return m_Data + index;
    }

    // Element Access
    reference operator[](size_t index) { return m_Data[index]; }
    const_reference operator[](size_t index) const { return m_Data[index]; }

    reference Get(size_t index)
    {
        if (index >= m_Size)
            throw std::out_of_range("TEArray index out of range");
        return m_Data[index];
    }
    const_reference Get(size_t index) const
    {
        if (index >= m_Size)
            throw std::out_of_range("TEArray index out of range");
        return m_Data[index];
    }
    reference at(size_t index) { return Get(index); }
    const_reference at(size_t index) const { return Get(index); }

    reference Front() { return m_Data[0]; }
    const_reference Front() const { return m_Data[0]; }
    reference front() { return m_Data[0]; }
    const_reference front() const { return m_Data[0]; }

    reference Back() { return m_Data[m_Size - 1]; }
    const_reference Back() const { return m_Data[m_Size - 1]; }
    reference back() { return m_Data[m_Size - 1]; }
    const_reference back() const { return m_Data[m_Size - 1]; }
    reference Last() { return m_Data[m_Size - 1]; }
    const_reference Last() const { return m_Data[m_Size - 1]; }
    reference last() { return m_Data[m_Size - 1]; }
    const_reference last() const { return m_Data[m_Size - 1]; }

    T *Data() noexcept { return m_Data; }
    const T *Data() const noexcept { return m_Data; }
    T *data() noexcept { return m_Data; }
    const T *data() const noexcept { return m_Data; }
    T *GetData() noexcept { return m_Data; }
    const T *GetData() const noexcept { return m_Data; }

    // Search & Equality
    bool Contains(const T &element) const
    {
        for (size_t i = 0; i < m_Size; ++i)
        {
            if (m_Data[i] == element)
                return true;
        }
        return false;
    }

    template <typename Predicate> bool ContainsBy(Predicate pred) const
    {
        for (size_t i = 0; i < m_Size; ++i)
        {
            if (pred(m_Data[i]))
                return true;
        }
        return false;
    }

    int IndexOf(const T &element) const
    {
        for (size_t i = 0; i < m_Size; ++i)
        {
            if (m_Data[i] == element)
                return static_cast<int>(i);
        }
        return -1;
    }

    template <typename Predicate> T *FindBy(Predicate pred)
    {
        for (size_t i = 0; i < m_Size; ++i)
        {
            if (pred(m_Data[i]))
                return &m_Data[i];
        }
        return nullptr;
    }

    template <typename Predicate> const T *FindBy(Predicate pred) const
    {
        for (size_t i = 0; i < m_Size; ++i)
        {
            if (pred(m_Data[i]))
                return &m_Data[i];
        }
        return nullptr;
    }

    // Removal
    bool Remove(const T &element)
    {
        int idx = IndexOf(element);
        if (idx != -1)
        {
            RemoveAt(static_cast<size_t>(idx));
            return true;
        }
        return false;
    }

    void RemoveAt(size_t index)
    {
        if (index >= m_Size)
            return;

        for (size_t i = index; i < m_Size - 1; ++i)
        {
            m_Data[i] = std::move(m_Data[i + 1]);
        }
        --m_Size;
        m_Data[m_Size].~T();
    }

    template <typename Predicate> size_t RemoveBy(Predicate pred)
    {
        size_t writeIdx = 0;
        size_t oldSize = m_Size;
        for (size_t readIdx = 0; readIdx < m_Size; ++readIdx)
        {
            if (!pred(m_Data[readIdx]))
            {
                if (writeIdx != readIdx)
                {
                    m_Data[writeIdx] = std::move(m_Data[readIdx]);
                }
                ++writeIdx;
            }
            else
            {
                m_Data[readIdx].~T();
            }
        }
        m_Size = writeIdx;
        return oldSize - m_Size;
    }

    iterator erase(const_iterator pos)
    {
        size_t index = static_cast<size_t>(pos - m_Data);
        RemoveAt(index);
        return m_Data + index;
    }

    iterator erase(const_iterator first, const_iterator last)
    {
        size_t startIdx = static_cast<size_t>(first - m_Data);
        size_t endIdx = static_cast<size_t>(last - m_Data);
        size_t count = endIdx - startIdx;
        if (count == 0)
            return m_Data + startIdx;

        for (size_t i = startIdx; i < endIdx; ++i)
        {
            m_Data[i].~T();
        }
        for (size_t i = endIdx; i < m_Size; ++i)
        {
            m_Data[i - count] = std::move(m_Data[i]);
        }
        m_Size -= count;
        return m_Data + startIdx;
    }

    void Empty() { Clear(); }
    void Clear()
    {
        DestroyRange(m_Data, m_Size);
        m_Size = 0;
    }
    void clear() { Clear(); }

    // Iterators
    iterator begin() noexcept { return m_Data; }
    iterator end() noexcept { return m_Data + m_Size; }
    const_iterator begin() const noexcept { return m_Data; }
    const_iterator end() const noexcept { return m_Data + m_Size; }
    const_iterator cbegin() const noexcept { return m_Data; }
    const_iterator cend() const noexcept { return m_Data + m_Size; }

    reverse_iterator rbegin() noexcept { return reverse_iterator(end()); }
    reverse_iterator rend() noexcept { return reverse_iterator(begin()); }
    const_reverse_iterator rbegin() const noexcept { return const_reverse_iterator(end()); }
    const_reverse_iterator rend() const noexcept { return const_reverse_iterator(begin()); }
    const_reverse_iterator crbegin() const noexcept { return const_reverse_iterator(cend()); }
    const_reverse_iterator crend() const noexcept { return const_reverse_iterator(cbegin()); }

    // Operators
    bool operator==(const TEArray<T> &other) const
    {
        if (m_Size != other.m_Size)
            return false;
        for (size_t i = 0; i < m_Size; ++i)
        {
            if (!(m_Data[i] == other.m_Data[i]))
                return false;
        }
        return true;
    }

    bool operator!=(const TEArray<T> &other) const { return !(*this == other); }
};


