#pragma once

// ====================================================================================
// Standard Library Includes
// ====================================================================================
// These headers provide core C++ STL functionality used across the engine.

#include <iostream>           // Standard I/O stream
#include <memory>             // Smart pointers (unique_ptr, shared_ptr)
#include <utility>            // Utilities (move, swap, etc.)
#include <algorithm>          // Algorithms (sort, max, etc.)
#include <functional>         // Functional utilities (function, bind)
#include <string>             // std::string operations
#include <vector>             // Dynamic array container
#include <unordered_map>      // Hash-based map
#include <map>                // Ordered map
#include <set>                // Ordered set
#include <unordered_set>      // Hash-based set
#include <fstream>            // File I/O
#include <sstream>            // String stream operations
#include <cassert>            // Assert macro
#include <chrono>             // Timing and profiling
#include <thread>             // Threading support
#include <mutex>              // Mutex for thread safety
#include <atomic>             // Atomic operations

// ====================================================================================
// Platform Detection & API Export Macros
// ====================================================================================
// Used to support DLL export/import on Windows.

#ifdef TE_PLATFORM_WINDOWS
    #ifdef TE_BUILD_DLL
        #define TE_API __declspec(dllexport)
    #else
        #define TE_API __declspec(dllimport)
    #endif
#else
    #error "TimeEngine only supports Windows currently!"
#endif

// ====================================================================================
// Common Macros
// ====================================================================================
// Bit masking macro to set a specific bit (used for flags, event categories, etc.)

#define BIT(x) (1 << x)

// ====================================================================================
// Delegate System
// ====================================================================================
// Provides a flexible event/delegate system using std::function and std::bind

// -------------------------------------
// Single-Cast Delegate Declaration
// Usage: DECLARE_DELEGATE(OnSomethingHappened)
// -------------------------------------
#define DECLARE_DELEGATE(Name) \
struct Name { \
    std::function<void()> Func; \
    void Bind(const std::function<void()>& f) { Func = f; } \
    void Execute() const { if (Func) Func(); } \
    void ExecuteIfBound() const { if (Func) Func(); } \
    bool IsBound() const { return static_cast<bool>(Func); } \
};

// -------------------------------------
// Multi-Cast Delegate Declaration
// Usage: DECLARE_MULTICAST_DELEGATE(OnSomethingBroadcast)
// -------------------------------------
#define DECLARE_MULTICAST_DELEGATE(Name) \
struct Name { \
    std::vector<std::function<void()>> Listeners; \
    void Add(const std::function<void()>& listener) { Listeners.push_back(listener); } \
    void Broadcast() const { for (auto& f : Listeners) f(); } \
    void Clear() { Listeners.clear(); } \
    bool IsBound() const { return !Listeners.empty(); } \
};

// ====================================================================================
// Binding Helpers
// ====================================================================================
// These macros simplify binding member functions and lambdas to delegates.
// Use SINGLE for DECLARE_DELEGATE and MULTI for DECLARE_MULTICAST_DELEGATE.

// -----------------------------
// Single-Cast Delegate Binders
// -----------------------------

// Bind non-const member function (no args)
#define BIND_FN_SINGLE(delegate, obj, func) \
    delegate.Bind(std::bind(&std::remove_reference<decltype(*obj)>::type::func, obj))

// Bind member function with 1 argument
#define BIND_FN_SINGLE_ARGS(delegate, obj, func) \
    delegate.Bind(std::bind(&std::remove_reference<decltype(*obj)>::type::func, obj, std::placeholders::_1))

// Bind const member function (no args)
#define BIND_FN_SINGLE_CONST(delegate, obj, func) \
    delegate.Bind(std::bind(&std::remove_reference<decltype(*obj)>::type::func, obj))

// Bind lambda or static function (no args)
#define BIND_LAMBDA_SINGLE(delegate, lambda) \
    delegate.Bind(std::function<void()>(lambda))

// -----------------------------
// Multi-Cast Delegate Binders
// -----------------------------

// Add non-const member function (no args)
#define BIND_FN_MULTI(delegate, obj, func) \
    delegate.Add(std::bind(&std::remove_reference<decltype(*obj)>::type::func, obj))

// Add member function with 1 argument
#define BIND_FN_MULTI_ARGS(delegate, obj, func) \
    delegate.Add(std::bind(&std::remove_reference<decltype(*obj)>::type::func, obj, std::placeholders::_1))

// Add const member function (no args)
#define BIND_FN_MULTI_CONST(delegate, obj, func) \
    delegate.Add(std::bind(&std::remove_reference<decltype(*obj)>::type::func, obj))

// Add lambda or static function (no args)
#define BIND_LAMBDA_MULTI(delegate, lambda) \
    delegate.Add(std::function<void()>(lambda))