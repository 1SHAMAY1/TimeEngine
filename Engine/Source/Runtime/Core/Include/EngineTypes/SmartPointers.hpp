#pragma once
#include <memory>
#include <utility>

// ====================================================================================
// Core Smart Pointer Aliases & Factory Functions (Global Namespace)
// ====================================================================================

// Unique Ownership (Exclusive, Move-Only)
template <typename T> using TEScope = std::unique_ptr<T>;

template <typename T, typename... Args> constexpr TEScope<T> CreateScope(Args &&...args)
{
    return std::make_unique<T>(std::forward<Args>(args)...);
}

// Shared Ownership (Reference Counted)
template <typename T> using TERef = std::shared_ptr<T>;

template <typename T, typename... Args> constexpr TERef<T> CreateRef(Args &&...args)
{
    return std::make_shared<T>(std::forward<Args>(args)...);
}

// Weak Non-Owning Observer (Breaks Reference Cycles)
template <typename T> using TEWeakRef = std::weak_ptr<T>;

// Common short aliases matching engine conventions
template <typename T> using Scope = TEScope<T>;
template <typename T> using Ref = TERef<T>;
template <typename T> using WeakRef = TEWeakRef<T>;
