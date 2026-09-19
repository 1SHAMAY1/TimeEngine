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

/// High-performance Linear / Bump Arena Allocator.
/// Provides O(1) instantaneous memory allocation and frame resets with zero OS calls.
class TE_API TEArenaAllocator
{
public:
    using Marker = size_t;

    TEArenaAllocator() noexcept = default;
    explicit TEArenaAllocator(size_t capacityBytes) { Initialize(capacityBytes); }

    ~TEArenaAllocator() { Shutdown(); }

    // Non-copyable, movable
    TEArenaAllocator(const TEArenaAllocator &) = delete;
    TEArenaAllocator &operator=(const TEArenaAllocator &) = delete;

    TEArenaAllocator(TEArenaAllocator &&other) noexcept
        : m_Buffer(other.m_Buffer), m_Capacity(other.m_Capacity), m_Offset(other.m_Offset),
          m_PeakUsage(other.m_PeakUsage)
    {
        other.m_Buffer = nullptr;
        other.m_Capacity = 0;
        other.m_Offset = 0;
        other.m_PeakUsage = 0;
    }

    TEArenaAllocator &operator=(TEArenaAllocator &&other) noexcept
    {
        if (this != &other)
        {
            Shutdown();
            m_Buffer = other.m_Buffer;
            m_Capacity = other.m_Capacity;
            m_Offset = other.m_Offset;
            m_PeakUsage = other.m_PeakUsage;

            other.m_Buffer = nullptr;
            other.m_Capacity = 0;
            other.m_Offset = 0;
            other.m_PeakUsage = 0;
        }
        return *this;
    }

    void Initialize(size_t capacityBytes)
    {
        if (m_Buffer)
            Shutdown();

        m_Capacity = capacityBytes;
        m_Offset = 0;
        m_PeakUsage = 0;
        if (m_Capacity > 0)
        {
            m_Buffer = static_cast<std::byte *>(std::malloc(m_Capacity));
        }
    }

    void Shutdown() noexcept
    {
        if (m_Buffer)
        {
            std::free(m_Buffer);
            m_Buffer = nullptr;
        }
        m_Capacity = 0;
        m_Offset = 0;
        m_PeakUsage = 0;
    }

    /// Allocate raw bytes with custom alignment (default: alignof(std::max_align_t))
    [[nodiscard]] void *Allocate(size_t sizeBytes, size_t alignment = alignof(std::max_align_t)) noexcept
    {
        if (!m_Buffer || sizeBytes == 0)
            return nullptr;

        const size_t currentAddress = reinterpret_cast<size_t>(m_Buffer) + m_Offset;
        const size_t alignedAddress = (currentAddress + (alignment - 1)) & ~(alignment - 1);
        const size_t padding = alignedAddress - currentAddress;

        if (m_Offset + padding + sizeBytes > m_Capacity)
            return nullptr; // Out of memory in this arena

        m_Offset += padding + sizeBytes;
        m_PeakUsage = (std::max)(m_PeakUsage, m_Offset);

        return reinterpret_cast<void *>(alignedAddress);
    }

    /// Typed object allocation helper
    template <typename T, typename... Args> [[nodiscard]] T *New(Args &&...args)
    {
        void *mem = Allocate(sizeof(T), alignof(T));
        if (!mem)
            return nullptr;
        return new (mem) T(std::forward<Args>(args)...);
    }

    /// Typed array allocation helper (trivial or caller-initialized)
    template <typename T> [[nodiscard]] T *NewArray(size_t count)
    {
        if (count == 0)
            return nullptr;
        void *mem = Allocate(sizeof(T) * count, alignof(T));
        if (!mem)
            return nullptr;
        if constexpr (!std::is_trivially_default_constructible_v<T>)
        {
            T *elements = static_cast<T *>(mem);
            for (size_t i = 0; i < count; ++i)
                new (&elements[i]) T();
        }
        return static_cast<T *>(mem);
    }

    /// Instantaneous O(1) reset
    void Reset() noexcept { m_Offset = 0; }

    /// Save a rewind marker
    [[nodiscard]] Marker GetMarker() const noexcept { return m_Offset; }

    /// Rewind memory to a saved marker
    void Rewind(Marker marker) noexcept
    {
        if (marker <= m_Capacity)
            m_Offset = marker;
    }

    [[nodiscard]] size_t GetAllocatedBytes() const noexcept { return m_Offset; }
    [[nodiscard]] size_t GetCapacity() const noexcept { return m_Capacity; }
    [[nodiscard]] size_t GetAvailableBytes() const noexcept
    {
        return m_Capacity > m_Offset ? m_Capacity - m_Offset : 0;
    }
    [[nodiscard]] size_t GetPeakUsage() const noexcept { return m_PeakUsage; }
    [[nodiscard]] bool IsInitialized() const noexcept { return m_Buffer != nullptr; }

private:
    std::byte *m_Buffer = nullptr;
    size_t m_Capacity = 0;
    size_t m_Offset = 0;
    size_t m_PeakUsage = 0;
};
