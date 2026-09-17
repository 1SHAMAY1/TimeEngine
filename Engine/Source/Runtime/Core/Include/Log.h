#pragma once

#include "CustomizableLogger.hpp"
#include "PreRequisites.h"
#include "EngineTypes/TEString.hpp"
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
    static void Init(bool logToFile = true, const TEString &file = "TimeEngineLog.json", bool silentBanner = false);

    static CustomizableLogger &GetCoreLogger();
    static CustomizableLogger &GetClientLogger();
    static bool HasCoreLogger();
    static bool HasClientLogger();

    static void AddMessage(const TEString &category, const TEString &message, const TEString &level);
    static TEArray<LogMessage> GetMessageBuffer();
    static void ClearMessageBuffer();
    static void SetProjectLogDirectory(const TEString &projectLogDir);
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
        if (Log::HasCoreLogger())                                                                                      \
            Log::GetCoreLogger().log("Core", msg, "INFO");                                                             \
        if (Log::ShouldLog("Core", "INFO"))                                                                            \
            Log::AddMessage("Core", msg, "INFO");                                                                      \
    } while (0)
#define TE_CORE_WARN_1(msg)                                                                                            \
    do                                                                                                                 \
    {                                                                                                                  \
        if (Log::HasCoreLogger())                                                                                      \
            Log::GetCoreLogger().log("Core", msg, "WARNING");                                                          \
        if (Log::ShouldLog("Core", "WARNING"))                                                                         \
            Log::AddMessage("Core", msg, "WARNING");                                                                   \
    } while (0)
#define TE_CORE_ERROR_1(msg)                                                                                           \
    do                                                                                                                 \
    {                                                                                                                  \
        if (Log::HasCoreLogger())                                                                                      \
            Log::GetCoreLogger().log("Core", msg, "ERROR");                                                            \
        if (Log::ShouldLog("Core", "ERROR"))                                                                           \
            Log::AddMessage("Core", msg, "ERROR");                                                                     \
    } while (0)
#define TE_CORE_CRITICAL_1(msg)                                                                                        \
    do                                                                                                                 \
    {                                                                                                                  \
        if (Log::HasCoreLogger())                                                                                      \
            Log::GetCoreLogger().log("Core", msg, "CRITICAL");                                                         \
        if (Log::ShouldLog("Core", "CRITICAL"))                                                                        \
            Log::AddMessage("Core", msg, "CRITICAL");                                                                  \
    } while (0)

// Support variadic (msg + args) - simplified to just stream them
#define TE_CORE_INFO(...)                                                                                              \
    do                                                                                                                 \
    {                                                                                                                  \
        TEString msg = LogFormat(__VA_ARGS__);                                                                         \
        if (Log::HasCoreLogger())                                                                                      \
            Log::GetCoreLogger().log("Core", msg, "INFO");                                                             \
        if (Log::ShouldLog("Core", "INFO"))                                                                            \
            Log::AddMessage("Core", msg, "INFO");                                                                      \
    } while (0)
#define TE_CORE_WARN(...)                                                                                              \
    do                                                                                                                 \
    {                                                                                                                  \
        TEString msg = LogFormat(__VA_ARGS__);                                                                         \
        if (Log::HasCoreLogger())                                                                                      \
            Log::GetCoreLogger().log("Core", msg, "WARNING");                                                          \
        if (Log::ShouldLog("Core", "WARNING"))                                                                         \
            Log::AddMessage("Core", msg, "WARNING");                                                                   \
    } while (0)
#define TE_CORE_ERROR(...)                                                                                             \
    do                                                                                                                 \
    {                                                                                                                  \
        TEString msg = LogFormat(__VA_ARGS__);                                                                         \
        if (Log::HasCoreLogger())                                                                                      \
            Log::GetCoreLogger().log("Core", msg, "ERROR");                                                            \
        if (Log::ShouldLog("Core", "ERROR"))                                                                           \
            Log::AddMessage("Core", msg, "ERROR");                                                                     \
    } while (0)
#define TE_CORE_DEBUG(...)                                                                                             \
    do                                                                                                                 \
    {                                                                                                                  \
        TEString msg = LogFormat(__VA_ARGS__);                                                                         \
        if (Log::HasCoreLogger())                                                                                      \
            Log::GetCoreLogger().log("Core", msg, "DEBUG");                                                            \
        if (Log::ShouldLog("Core", "DEBUG"))                                                                            \
            Log::AddMessage("Core", msg, "DEBUG");                                                                     \
    } while (0)
#define TE_INPUT_DEBUG(...)                                                                                            \
    do                                                                                                                 \
    {                                                                                                                  \
        TEString msg = LogFormat(__VA_ARGS__);                                                                         \
        if (Log::HasCoreLogger())                                                                                      \
            Log::GetCoreLogger().log("Input", msg, "DEBUG");                                                           \
        if (Log::ShouldLog("Input", "DEBUG"))                                                                          \
            Log::AddMessage("Input", msg, "DEBUG");                                                                    \
    } while (0)
#define TE_CORE_CRITICAL(...)                                                                                          \
    do                                                                                                                 \
    {                                                                                                                  \
        TEString msg = LogFormat(__VA_ARGS__);                                                                         \
        if (Log::HasCoreLogger())                                                                                      \
            Log::GetCoreLogger().log("Core", msg, "CRITICAL");                                                         \
        if (Log::ShouldLog("Core", "CRITICAL"))                                                                        \
            Log::AddMessage("Core", msg, "CRITICAL");                                                                  \
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
        TEString msg = LogFormat(__VA_ARGS__);                                                                         \
        if (Log::HasClientLogger())                                                                                    \
            Log::GetClientLogger().log("Client", msg, "INFO");                                                         \
        if (Log::ShouldLog("Client", "INFO"))                                                                          \
            Log::AddMessage("Client", msg, "INFO");                                                                    \
    } while (0)
#define TE_CLIENT_WARN(...)                                                                                            \
    do                                                                                                                 \
    {                                                                                                                  \
        TEString msg = LogFormat(__VA_ARGS__);                                                                         \
        if (Log::HasClientLogger())                                                                                    \
            Log::GetClientLogger().log("Client", msg, "WARNING");                                                      \
        if (Log::ShouldLog("Client", "WARNING"))                                                                       \
            Log::AddMessage("Client", msg, "WARNING");                                                                 \
    } while (0)
#define TE_CLIENT_ERROR(...)                                                                                           \
    do                                                                                                                 \
    {                                                                                                                  \
        TEString msg = LogFormat(__VA_ARGS__);                                                                         \
        if (Log::HasClientLogger())                                                                                    \
            Log::GetClientLogger().log("Client", msg, "ERROR");                                                        \
        if (Log::ShouldLog("Client", "ERROR"))                                                                         \
            Log::AddMessage("Client", msg, "ERROR");                                                                   \
    } while (0)
#define TE_CLIENT_DEBUG(...)                                                                                           \
    do                                                                                                                 \
    {                                                                                                                  \
        TEString msg = LogFormat(__VA_ARGS__);                                                                         \
        if (Log::HasClientLogger())                                                                                    \
            Log::GetClientLogger().log("Client", msg, "DEBUG");                                                        \
        if (Log::ShouldLog("Client", "DEBUG"))                                                                         \
            Log::AddMessage("Client", msg, "DEBUG");                                                                   \
    } while (0)
#define TE_CLIENT_CRITICAL(...)                                                                                        \
    do                                                                                                                 \
    {                                                                                                                  \
        TEString msg = LogFormat(__VA_ARGS__);                                                                         \
        if (Log::HasClientLogger())                                                                                    \
            Log::GetClientLogger().log("Client", msg, "CRITICAL");                                                     \
        if (Log::ShouldLog("Client", "CRITICAL"))                                                                        \
            Log::AddMessage("Client", msg, "CRITICAL");                                                                \
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
        if (Log::ShouldLog("Core", "ERROR"))                                                                           \
        {                                                                                                              \
            Log::AddMessage("Core", msg, "ERROR");                                                                     \
            Log::GetCoreLogger().log("Core", msg, "ERROR");                                                            \
        }                                                                                                              \
    } while (0)
#define TE_CORE_CRITICAL_1(msg)                                                                                        \
    do                                                                                                                 \
    {                                                                                                                  \
        if (Log::ShouldLog("Core", "CRITICAL"))                                                                        \
        {                                                                                                              \
            Log::AddMessage("Core", msg, "CRITICAL");                                                                  \
            Log::GetCoreLogger().log("Core", msg, "CRITICAL");                                                         \
        }                                                                                                              \
    } while (0)

#define TE_CORE_INFO(...)
#define TE_CORE_WARN(...)
#define TE_CORE_ERROR(...)                                                                                             \
    do                                                                                                                 \
    {                                                                                                                  \
        if (Log::ShouldLog("Core", "ERROR"))                                                                           \
        {                                                                                                              \
            TEString msg = LogFormat(__VA_ARGS__);                                                                     \
            Log::AddMessage("Core", msg, "ERROR");                                                                     \
            Log::GetCoreLogger().log("Core", msg, "ERROR");                                                            \
        }                                                                                                              \
    } while (0)
#define TE_CORE_DEBUG(...)
#define TE_INPUT_DEBUG(...)
#define TE_CORE_CRITICAL(...)                                                                                          \
    do                                                                                                                 \
    {                                                                                                                  \
        if (Log::ShouldLog("Core", "CRITICAL"))                                                                        \
        {                                                                                                              \
            TEString msg = LogFormat(__VA_ARGS__);                                                                     \
            Log::AddMessage("Core", msg, "CRITICAL");                                                                  \
            Log::GetCoreLogger().log("Core", msg, "CRITICAL");                                                         \
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
        if (Log::ShouldLog("Client", "ERROR"))                                                                         \
        {                                                                                                              \
            TEString msg = LogFormat(__VA_ARGS__);                                                                     \
            Log::AddMessage("Client", msg, "ERROR");                                                                   \
            Log::GetClientLogger().log("Client", msg, "ERROR");                                                        \
        }                                                                                                              \
    } while (0)
#define TE_CLIENT_DEBUG(...)
#define TE_CLIENT_CRITICAL(...)                                                                                        \
    do                                                                                                                 \
    {                                                                                                                  \
        if (Log::ShouldLog("Client", "CRITICAL"))                                                                      \
        {                                                                                                              \
            TEString msg = LogFormat(__VA_ARGS__);                                                                     \
            Log::AddMessage("Client", msg, "CRITICAL");                                                                \
            Log::GetClientLogger().log("Client", msg, "CRITICAL");                                                     \
        }                                                                                                              \
    } while (0)
#define TE_CLIENT_ASSERT(x, msg)                                                                                       \
    if (!(x))                                                                                                          \
    {                                                                                                                  \
        TE_CLIENT_CRITICAL(msg);                                                                                       \
        __debugbreak();                                                                                                \
    }
#endif
