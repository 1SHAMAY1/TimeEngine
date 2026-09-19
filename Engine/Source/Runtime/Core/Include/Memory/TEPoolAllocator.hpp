#pragma once
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <algorithm>
#include <utility>
#include <new>
#include <type_traits>

#ifndef TE_API
#ifdef TE_PLATFORM_WINDOWS
#ifdef TE_BUILD_DLL
#define TE_API __declspec(dllexport)
#else
#define TE_API __declspec(dllimport)
#endif
#else
#define TE_API __attribute__((visibility("default")))
#endif
#endif

/// High-performance Fixed-Size Block Pool Allocator.
/// Eliminates memory fragmentation and OS heap locks for frequent allocations of objects of uniform size.
class TE_API TEPoolAllocator
{
private:
    struct FreeNode
    {
        FreeNode *Next;
    };

public:
    TEPoolAllocator() noexcept = default;
    TEPoolAllocator(size_t objectSize, size_t objectCount, size_t alignment = alignof(std::max_align_t))
    {
        Initialize(objectSize, objectCount, alignment);
    }

    ~TEPoolAllocator() { Shutdown(); }

    TEPoolAllocator(const TEPoolAllocator &) = delete;
    TEPoolAllocator &operator=(const TEPoolAllocator &) = delete;

    TEPoolAllocator(TEPoolAllocator &&other) noexcept
        : m_Buffer(other.m_Buffer), m_FreeList(other.m_FreeList), m_ObjectSize(other.m_ObjectSize),
          m_BlockSize(other.m_BlockSize), m_ObjectCount(other.m_ObjectCount), m_AllocatedCount(other.m_AllocatedCount),
          m_PeakAllocatedCount(other.m_PeakAllocatedCount)
    {
        other.m_Buffer = nullptr;
        other.m_FreeList = nullptr;
        other.m_ObjectSize = 0;
        other.m_BlockSize = 0;
        other.m_ObjectCount = 0;
        other.m_AllocatedCount = 0;
        other.m_PeakAllocatedCount = 0;
    }

    TEPoolAllocator &operator=(TEPoolAllocator &&other) noexcept
    {
        if (this != &other)
        {
            Shutdown();
            m_Buffer = other.m_Buffer;
            m_FreeList = other.m_FreeList;
            m_ObjectSize = other.m_ObjectSize;
            m_BlockSize = other.m_BlockSize;
            m_ObjectCount = other.m_ObjectCount;
            m_AllocatedCount = other.m_AllocatedCount;
            m_PeakAllocatedCount = other.m_PeakAllocatedCount;

            other.m_Buffer = nullptr;
            other.m_FreeList = nullptr;
            other.m_ObjectSize = 0;
            other.m_BlockSize = 0;
            other.m_ObjectCount = 0;
            other.m_AllocatedCount = 0;
            other.m_PeakAllocatedCount = 0;
        }
        return *this;
    }

    void Initialize(size_t objectSize, size_t objectCount, size_t alignment = alignof(std::max_align_t))
    {
        if (m_Buffer)
            Shutdown();

        m_ObjectSize = objectSize;
        m_ObjectCount = objectCount;
        m_AllocatedCount = 0;
        m_PeakAllocatedCount = 0;

        const size_t minBlockSize = (std::max)(sizeof(FreeNode), objectSize);
        m_BlockSize = (minBlockSize + (alignment - 1)) & ~(alignment - 1);

        if (m_BlockSize > 0 && m_ObjectCount > 0)
        {
            m_Buffer = static_cast<std::byte *>(std::malloc(m_BlockSize * m_ObjectCount));
            Reset();
        }
    }

    void Shutdown() noexcept
    {
        if (m_Buffer)
        {
            std::free(m_Buffer);
            m_Buffer = nullptr;
        }
        m_FreeList = nullptr;
        m_ObjectSize = 0;
        m_BlockSize = 0;
        m_ObjectCount = 0;
        m_AllocatedCount = 0;
        m_PeakAllocatedCount = 0;
    }

    /// Allocate single block from pool (O(1))
    [[nodiscard]] void *Allocate() noexcept
    {
        if (!m_FreeList)
            return nullptr; // Out of pool capacity

        FreeNode *node = m_FreeList;
        m_FreeList = m_FreeList->Next;
        ++m_AllocatedCount;
        m_PeakAllocatedCount = (std::max)(m_PeakAllocatedCount, m_AllocatedCount);

        return reinterpret_cast<void *>(node);
    }

    /// Free block back to pool (O(1))
    void Free(void *ptr) noexcept
    {
        if (!ptr || !m_Buffer)
            return;

        const size_t bufferStart = reinterpret_cast<size_t>(m_Buffer);
        const size_t bufferEnd = bufferStart + (m_BlockSize * m_ObjectCount);
        const size_t address = reinterpret_cast<size_t>(ptr);

        // Sanity check that pointer is in pool range
        if (address < bufferStart || address >= bufferEnd)
            return;

        FreeNode *node = reinterpret_cast<FreeNode *>(ptr);
        node->Next = m_FreeList;
        m_FreeList = node;

        if (m_AllocatedCount > 0)
            --m_AllocatedCount;
    }

    /// Typed allocation helper
    template <typename T, typename... Args> [[nodiscard]] T *New(Args &&...args)
    {
        static_assert(sizeof(T) <= 4096, "T is unusually large for TEPoolAllocator.");
        void *mem = Allocate();
        if (!mem)
            return nullptr;
        return new (mem) T(std::forward<Args>(args)...);
    }

    /// Typed destruction helper
    template <typename T> void Delete(T *ptr)
    {
        if (!ptr)
            return;
        ptr->~T();
        Free(ptr);
    }

    /// Rebuild free list to initial empty state
    void Reset() noexcept
    {
        if (!m_Buffer || m_BlockSize == 0 || m_ObjectCount == 0)
            return;

        m_FreeList = reinterpret_cast<FreeNode *>(m_Buffer);
        FreeNode *current = m_FreeList;

        for (size_t i = 0; i < m_ObjectCount - 1; ++i)
        {
            std::byte *nextAddress = reinterpret_cast<std::byte *>(current) + m_BlockSize;
            current->Next = reinterpret_cast<FreeNode *>(nextAddress);
            current = current->Next;
        }
        current->Next = nullptr;
        m_AllocatedCount = 0;
    }

    [[nodiscard]] size_t GetObjectSize() const noexcept { return m_ObjectSize; }
    [[nodiscard]] size_t GetBlockSize() const noexcept { return m_BlockSize; }
    [[nodiscard]] size_t GetCapacity() const noexcept { return m_ObjectCount; }
    [[nodiscard]] size_t GetAllocatedCount() const noexcept { return m_AllocatedCount; }
    [[nodiscard]] size_t GetFreeCount() const noexcept
    {
        return m_ObjectCount > m_AllocatedCount ? m_ObjectCount - m_AllocatedCount : 0;
    }
    [[nodiscard]] size_t GetPeakAllocatedCount() const noexcept { return m_PeakAllocatedCount; }
    [[nodiscard]] bool IsInitialized() const noexcept { return m_Buffer != nullptr; }

private:
    std::byte *m_Buffer = nullptr;
    FreeNode *m_FreeList = nullptr;
    size_t m_ObjectSize = 0;
    size_t m_BlockSize = 0;
    size_t m_ObjectCount = 0;
    size_t m_AllocatedCount = 0;
    size_t m_PeakAllocatedCount = 0;
};

/// Templated typed pool allocator for a specific class
template <typename T, size_t Count = 1024> class TTypedPool
{
public:
    TTypedPool() : m_Pool(sizeof(T), Count, alignof(T)) {}

    template <typename... Args> [[nodiscard]] T *Allocate(Args &&...args)
    {
        return m_Pool.New<T>(std::forward<Args>(args)...);
    }

    void Free(T *ptr) { m_Pool.Delete<T>(ptr); }

    void Reset() { m_Pool.Reset(); }

    [[nodiscard]] size_t GetAllocatedCount() const noexcept { return m_Pool.GetAllocatedCount(); }
    [[nodiscard]] size_t GetCapacity() const noexcept { return m_Pool.GetCapacity(); }

private:
    TEPoolAllocator m_Pool;
};
