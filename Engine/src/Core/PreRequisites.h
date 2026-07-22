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
#include <queue>              // Queue operations

#ifdef __MINGW32__
#include <sec_api/string_s.h>
#endif


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
    #define TE_API __attribute__((visibility("default")))
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


#pragma once

// === [Meta Marker Macros for Classes, Structs, Enums] ===
// NOTE: Parameters like Category, Name, Description, Tooltip are for future parsing

#define TE_CLASS(...)                                          \
static constexpr const char* TEClassCategory    = "DefaultClass"; \
static constexpr const char* TEClassName        = "UnnamedClass"; \
static constexpr const char* TEClassDescription = "No description provided."; \
static constexpr const char* TEClassTooltip     = "No tooltip provided.";

#define TE_STRUCT(...)                                          \
static constexpr const char* TEStructCategory    = "DefaultStruct"; \
static constexpr const char* TEStructName        = "UnnamedStruct"; \
static constexpr const char* TEStructDescription = "No description provided."; \
static constexpr const char* TEStructTooltip     = "No tooltip provided."

#define TE_ENUM(...)                                          \
static constexpr const char* TEEnumCategory    = "DefaultEnum"; \
static constexpr const char* TEEnumName        = "UnnamedEnum"; \
static constexpr const char* TEEnumDescription = "No description provided."; \
static constexpr const char* TEEnumTooltip     = "No tooltip provided."

// === [Property Marker Macro] ===
// Marker only (zero runtime overhead)
#define TEPROPERTY(...) [[maybe_unused]]

// ====================================================================================
// Cross-Platform Compiler Helpers
// ====================================================================================
#ifndef _WIN32
#include <cstring>
template <size_t Size>
inline int strcpy_s(char (&dest)[Size], const char* src) {
    if (src == nullptr) return 22; // EINVAL
    size_t src_len = std::strlen(src);
    if (src_len >= Size) {
        if (Size > 0) dest[0] = '\0';
        return 34; // ERANGE
    }
    std::strncpy(dest, src, Size - 1);
    dest[src_len] = '\0';
    return 0;
}

template <size_t Size>
inline int strncpy_s(char (&dest)[Size], const char* src, size_t count) {
    if (src == nullptr) return 22; // EINVAL
    size_t copy_len = count < Size ? count : Size - 1;
    std::strncpy(dest, src, copy_len);
    dest[copy_len] = '\0';
    return 0;
}
#endif

#ifndef _MSC_VER
    #if defined(__i386__) || defined(__x86_64__)
        #define __debugbreak() __asm__ volatile("int $3")
    #elif defined(__aarch64__)
        #define __debugbreak() __asm__ volatile("brk #0xf000")
    #elif defined(__arm__)
        #define __debugbreak() __asm__ volatile("udf #254")
    #else
        #include <signal.h>
        #define __debugbreak() raise(SIGTRAP)
    #endif
#endif

// ====================================================================================
// Centralized Cross-Platform OS Workarounds
// ====================================================================================
#ifdef TE_PLATFORM_WINDOWS
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
    #ifndef NOMINMAX
        #define NOMINMAX
    #endif
    #include <Windows.h>
    #ifdef ERROR
        #undef ERROR
    #endif
    #ifdef GetClassName
        #undef GetClassName
    #endif
#endif

#if defined(__APPLE__) && !defined(__clang__)
    // GCC on macOS compatibility fix for SDK assertions
    #define xnu_static_assert_struct_size(...)
    #define xnu_static_assert_struct_size_kernel_user(...)
    #define xnu_static_assert_struct_size_kernel_user64_user32(...)
#endif

#ifndef TE_PLATFORM_WINDOWS
    #include <dlfcn.h>
    using HMODULE = void*;
    #define GetProcAddress dlsym
    #define FreeLibrary dlclose

    inline HMODULE LoadLibraryW(const wchar_t* wpath) {
        std::wstring ws(wpath);
        std::string path(ws.begin(), ws.end());
        return dlopen(path.c_str(), RTLD_NOW);
    }
#endif

// ====================================================================================
// Centralized Graphics API Support Detection
// ====================================================================================
#ifdef TE_PLATFORM_WINDOWS
    #define TE_SUPPORT_OPENGL
    #define TE_SUPPORT_DIRECTX11
    #define TE_SUPPORT_VULKAN
#elif defined(__APPLE__)
    #define TE_SUPPORT_METAL
    // OpenGL, Vulkan, and DX11 are excluded or unsupported on macOS
#else // Linux
    #define TE_SUPPORT_OPENGL
    #define TE_SUPPORT_VULKAN
#endif


