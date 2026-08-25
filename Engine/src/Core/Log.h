#pragma once

#include "CustomizableLogger.hpp"
#include "PreRequisites.h"
#include "GameFrameWork/GameplayUtils.hpp"
#include "Utils/TEString.hpp"
#include <sstream>

struct LogMessage
{
    TEString Timestamp;
    TEString Category;
    TEString Message;
    TEString Level;
};

class TE_API Log
{
public:
    static void Init(bool logToFile = true, const TEString &file = "TimeEngineLog.json");

    inline static CustomizableLogger &GetCoreLogger() { return *s_CoreLogger; }
    inline static CustomizableLogger &GetClientLogger() { return *s_ClientLogger; }
    inline static bool HasCoreLogger() { return s_CoreLogger != nullptr; }
    inline static bool HasClientLogger() { return s_ClientLogger != nullptr; }

    static void AddMessage(const TEString &category, const TEString &message, const TEString &level);
    static TEArray<LogMessage> GetMessageBuffer();
    static void ClearMessageBuffer();
    static bool ShouldLog(const TEString &category, const TEString &level);
    static TEVector4 GetLogColor(const TEString &category, const TEString &level);

private:
    static TEScope<CustomizableLogger> s_CoreLogger;
    static TEScope<CustomizableLogger> s_ClientLogger;
};

// Simple variadic string joiner for logs to avoid fmt dependency issues
template <typename... Args> TEString LogFormat(Args &&...args)
{
    std::ostringstream ss;
    (ss << ... << args);
    return ss.str();
}

#ifndef TE_MINIMIZED
// Support single argument (msg only)
#define TE_CORE_INFO_1(msg)                                                                                            \
    do                                                                                                                 \
    {                                                                                                                  \
        if (Log::ShouldLog("Core", "INFO"))                                                                      \
        {                                                                                                              \
            Log::AddMessage("Core", msg, "INFO");                                                                \
            if (Log::HasCoreLogger())                                                                            \
                Log::GetCoreLogger().log("Core", msg, "INFO");                                                   \
        }                                                                                                              \
    } while (0)
#define TE_CORE_WARN_1(msg)                                                                                            \
    do                                                                                                                 \
    {                                                                                                                  \
        if (Log::ShouldLog("Core", "WARNING"))                                                                   \
        {                                                                                                              \
            Log::AddMessage("Core", msg, "WARNING");                                                             \
            if (Log::HasCoreLogger())                                                                            \
                Log::GetCoreLogger().log("Core", msg, "WARNING");                                                \
        }                                                                                                              \
    } while (0)
#define TE_CORE_ERROR_1(msg)                                                                                           \
    do                                                                                                                 \
    {                                                                                                                  \
        if (Log::ShouldLog("Core", "ERROR"))                                                                     \
        {                                                                                                              \
            Log::AddMessage("Core", msg, "ERROR");                                                               \
            if (Log::HasCoreLogger())                                                                            \
                Log::GetCoreLogger().log("Core", msg, "ERROR");                                                  \
        }                                                                                                              \
    } while (0)
#define TE_CORE_CRITICAL_1(msg)                                                                                        \
    do                                                                                                                 \
    {                                                                                                                  \
        if (Log::ShouldLog("Core", "CRITICAL"))                                                                  \
        {                                                                                                              \
            Log::AddMessage("Core", msg, "CRITICAL");                                                            \
            if (Log::HasCoreLogger())                                                                            \
                Log::GetCoreLogger().log("Core", msg, "CRITICAL");                                               \
        }                                                                                                              \
    } while (0)

// Support variadic (msg + args) - simplified to just stream them
#define TE_CORE_INFO(...)                                                                                              \
    do                                                                                                                 \
    {                                                                                                                  \
        if (Log::ShouldLog("Core", "INFO"))                                                                      \
        {                                                                                                              \
            TEString msg = LogFormat(__VA_ARGS__);                                                                  \
            Log::AddMessage("Core", msg, "INFO");                                                                \
            if (Log::HasCoreLogger())                                                                            \
                Log::GetCoreLogger().log("Core", msg, "INFO");                                                   \
        }                                                                                                              \
    } while (0)
#define TE_CORE_WARN(...)                                                                                              \
    do                                                                                                                 \
    {                                                                                                                  \
        if (Log::ShouldLog("Core", "WARNING"))                                                                   \
        {                                                                                                              \
            TEString msg = LogFormat(__VA_ARGS__);                                                                  \
            Log::AddMessage("Core", msg, "WARNING");                                                             \
            if (Log::HasCoreLogger())                                                                            \
                Log::GetCoreLogger().log("Core", msg, "WARNING");                                                \
        }                                                                                                              \
    } while (0)
#define TE_CORE_ERROR(...)                                                                                             \
    do                                                                                                                 \
    {                                                                                                                  \
        if (Log::ShouldLog("Core", "ERROR"))                                                                     \
        {                                                                                                              \
            TEString msg = LogFormat(__VA_ARGS__);                                                                  \
            Log::AddMessage("Core", msg, "ERROR");                                                               \
            if (Log::HasCoreLogger())                                                                            \
                Log::GetCoreLogger().log("Core", msg, "ERROR");                                                  \
        }                                                                                                              \
    } while (0)
#define TE_CORE_DEBUG(...)                                                                                             \
    do                                                                                                                 \
    {                                                                                                                  \
        if (Log::ShouldLog("Core", "DEBUG"))                                                                     \
        {                                                                                                              \
            TEString msg = LogFormat(__VA_ARGS__);                                                                  \
            Log::AddMessage("Core", msg, "DEBUG");                                                               \
            if (Log::HasCoreLogger())                                                                            \
                Log::GetCoreLogger().log("Core", msg, "DEBUG");                                                  \
        }                                                                                                              \
    } while (0)
#define TE_INPUT_DEBUG(...)                                                                                            \
    do                                                                                                                 \
    {                                                                                                                  \
        if (Log::ShouldLog("Input", "DEBUG"))                                                                    \
        {                                                                                                              \
            TEString msg = LogFormat(__VA_ARGS__);                                                                  \
            Log::AddMessage("Input", msg, "DEBUG");                                                              \
            if (Log::HasCoreLogger())                                                                            \
                Log::GetCoreLogger().log("Input", msg, "DEBUG");                                                 \
        }                                                                                                              \
    } while (0)
#define TE_CORE_CRITICAL(...)                                                                                          \
    do                                                                                                                 \
    {                                                                                                                  \
        if (Log::ShouldLog("Core", "CRITICAL"))                                                                  \
        {                                                                                                              \
            TEString msg = LogFormat(__VA_ARGS__);                                                                  \
            Log::AddMessage("Core", msg, "CRITICAL");                                                            \
            if (Log::HasCoreLogger())                                                                            \
                Log::GetCoreLogger().log("Core", msg, "CRITICAL");                                               \
        }                                                                                                              \
    } while (0)

#define TE_CORE_ASSERT(x, msg)                                                                                         \
    if (!(x))                                                                                                          \
    {                                                                                                                  \
        TE_CORE_CRITICAL(msg);                                                                                         \
        __debugbreak();                                                                                                \
    }

#define TE_CLIENT_INFO(...)                                                                                            \
    do                                                                                                                 \
    {                                                                                                                  \
        if (Log::ShouldLog("Client", "INFO"))                                                                    \
        {                                                                                                              \
            TEString msg = LogFormat(__VA_ARGS__);                                                                  \
            Log::AddMessage("Client", msg, "INFO");                                                              \
            if (Log::HasClientLogger())                                                                          \
                Log::GetClientLogger().log("Client", msg, "INFO");                                               \
        }                                                                                                              \
    } while (0)
#define TE_CLIENT_WARN(...)                                                                                            \
    do                                                                                                                 \
    {                                                                                                                  \
        if (Log::ShouldLog("Client", "WARNING"))                                                                 \
        {                                                                                                              \
            TEString msg = LogFormat(__VA_ARGS__);                                                                  \
            Log::AddMessage("Client", msg, "WARNING");                                                           \
            if (Log::HasClientLogger())                                                                          \
                Log::GetClientLogger().log("Client", msg, "WARNING");                                            \
        }                                                                                                              \
    } while (0)
#define TE_CLIENT_ERROR(...)                                                                                           \
    do                                                                                                                 \
    {                                                                                                                  \
        if (Log::ShouldLog("Client", "ERROR"))                                                                   \
        {                                                                                                              \
            TEString msg = LogFormat(__VA_ARGS__);                                                                  \
            Log::AddMessage("Client", msg, "ERROR");                                                             \
            if (Log::HasClientLogger())                                                                          \
                Log::GetClientLogger().log("Client", msg, "ERROR");                                              \
        }                                                                                                              \
    } while (0)
#define TE_CLIENT_DEBUG(...)                                                                                           \
    do                                                                                                                 \
    {                                                                                                                  \
        if (Log::ShouldLog("Client", "DEBUG"))                                                                   \
        {                                                                                                              \
            TEString msg = LogFormat(__VA_ARGS__);                                                                  \
            Log::AddMessage("Client", msg, "DEBUG");                                                             \
            if (Log::HasClientLogger())                                                                          \
                Log::GetClientLogger().log("Client", msg, "DEBUG");                                              \
        }                                                                                                              \
    } while (0)
#define TE_CLIENT_CRITICAL(...)                                                                                        \
    do                                                                                                                 \
    {                                                                                                                  \
        if (Log::ShouldLog("Client", "CRITICAL"))                                                                \
        {                                                                                                              \
            TEString msg = LogFormat(__VA_ARGS__);                                                                  \
            Log::AddMessage("Client", msg, "CRITICAL");                                                          \
            if (Log::HasClientLogger())                                                                          \
                Log::GetClientLogger().log("Client", msg, "CRITICAL");                                           \
        }                                                                                                              \
    } while (0)
#define TE_CLIENT_ASSERT(x, msg)                                                                                       \
    if (!(x))                                                                                                          \
    {                                                                                                                  \
        TE_CLIENT_CRITICAL(msg);                                                                                       \
        __debugbreak();                                                                                                \
    }
#else
// Minimized build logic: Strip INFO, WARN, and DEBUG, but keep ERROR/CRITICAL/ASSERT
#define TE_CORE_INFO_1(msg)
#define TE_CORE_WARN_1(msg)
#define TE_CORE_ERROR_1(msg)                                                                                           \
    do                                                                                                                 \
    {                                                                                                                  \
        if (Log::ShouldLog("Core", "ERROR"))                                                                     \
        {                                                                                                              \
            Log::AddMessage("Core", msg, "ERROR");                                                               \
            Log::GetCoreLogger().log("Core", msg, "ERROR");                                                      \
        }                                                                                                              \
    } while (0)
#define TE_CORE_CRITICAL_1(msg)                                                                                        \
    do                                                                                                                 \
    {                                                                                                                  \
        if (Log::ShouldLog("Core", "CRITICAL"))                                                                  \
        {                                                                                                              \
            Log::AddMessage("Core", msg, "CRITICAL");                                                            \
            Log::GetCoreLogger().log("Core", msg, "CRITICAL");                                                   \
        }                                                                                                              \
    } while (0)

#define TE_CORE_INFO(...)
#define TE_CORE_WARN(...)
#define TE_CORE_ERROR(...)                                                                                             \
    do                                                                                                                 \
    {                                                                                                                  \
        if (Log::ShouldLog("Core", "ERROR"))                                                                     \
        {                                                                                                              \
            TEString msg = LogFormat(__VA_ARGS__);                                                                  \
            Log::AddMessage("Core", msg, "ERROR");                                                               \
            Log::GetCoreLogger().log("Core", msg, "ERROR");                                                      \
        }                                                                                                              \
    } while (0)
#define TE_CORE_DEBUG(...)
#define TE_INPUT_DEBUG(...)
#define TE_CORE_CRITICAL(...)                                                                                          \
    do                                                                                                                 \
    {                                                                                                                  \
        if (Log::ShouldLog("Core", "CRITICAL"))                                                                  \
        {                                                                                                              \
            TEString msg = LogFormat(__VA_ARGS__);                                                                  \
            Log::AddMessage("Core", msg, "CRITICAL");                                                            \
            Log::GetCoreLogger().log("Core", msg, "CRITICAL");                                                   \
        }                                                                                                              \
    } while (0)

#define TE_CORE_ASSERT(x, msg)                                                                                         \
    if (!(x))                                                                                                          \
    {                                                                                                                  \
        TE_CORE_CRITICAL(msg);                                                                                         \
        __debugbreak();                                                                                                \
    }

#define TE_CLIENT_INFO(...)
#define TE_CLIENT_WARN(...)
#define TE_CLIENT_ERROR(...)                                                                                           \
    do                                                                                                                 \
    {                                                                                                                  \
        if (Log::ShouldLog("Client", "ERROR"))                                                                   \
        {                                                                                                              \
            TEString msg = LogFormat(__VA_ARGS__);                                                                  \
            Log::AddMessage("Client", msg, "ERROR");                                                             \
            Log::GetClientLogger().log("Client", msg, "ERROR");                                                  \
        }                                                                                                              \
    } while (0)
#define TE_CLIENT_DEBUG(...)
#define TE_CLIENT_CRITICAL(...)                                                                                        \
    do                                                                                                                 \
    {                                                                                                                  \
        if (Log::ShouldLog("Client", "CRITICAL"))                                                                \
        {                                                                                                              \
            TEString msg = LogFormat(__VA_ARGS__);                                                                  \
            Log::AddMessage("Client", msg, "CRITICAL");                                                          \
            Log::GetClientLogger().log("Client", msg, "CRITICAL");                                               \
        }                                                                                                              \
    } while (0)
#define TE_CLIENT_ASSERT(x, msg)                                                                                       \
    if (!(x))                                                                                                          \
    {                                                                                                                  \
        TE_CLIENT_CRITICAL(msg);                                                                                       \
        __debugbreak();                                                                                                \
    }
#endif

