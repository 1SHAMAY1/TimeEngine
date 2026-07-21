#pragma once

#include "CustomizableLogger.hpp"
#include "PreRequisites.h"
#include <sstream>
#include <string>
#include <vector>

namespace TE
{
struct LogMessage
{
    std::string Timestamp;
    std::string Category;
    std::string Message;
    std::string Level;
};

class TE_API Log
{
public:
    static void Init(bool logToFile = true, const std::string &file = "TimeEngineLog.json");

    inline static CustomizableLogger &GetCoreLogger() { return *s_CoreLogger; }
    inline static CustomizableLogger &GetClientLogger() { return *s_ClientLogger; }

    static void AddMessage(const std::string &category, const std::string &message, const std::string &level);
    static std::vector<LogMessage> GetMessageBuffer();
    static void ClearMessageBuffer();
    static bool ShouldLog(const std::string &category, const std::string &level);

private:
    static std::unique_ptr<CustomizableLogger> s_CoreLogger;
    static std::unique_ptr<CustomizableLogger> s_ClientLogger;
};
} // namespace TE

// Simple variadic string joiner for logs to avoid fmt dependency issues
template <typename... Args> std::string LogFormat(Args &&...args)
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
        if (::TE::Log::ShouldLog("Core", "INFO"))                                                                      \
        {                                                                                                              \
            ::TE::Log::AddMessage("Core", msg, "INFO");                                                                \
            ::TE::Log::GetCoreLogger().log("Core", msg, "INFO");                                                       \
        }                                                                                                              \
    } while (0)
#define TE_CORE_WARN_1(msg)                                                                                            \
    do                                                                                                                 \
    {                                                                                                                  \
        if (::TE::Log::ShouldLog("Core", "WARNING"))                                                                   \
        {                                                                                                              \
            ::TE::Log::AddMessage("Core", msg, "WARNING");                                                             \
            ::TE::Log::GetCoreLogger().log("Core", msg, "WARNING");                                                    \
        }                                                                                                              \
    } while (0)
#define TE_CORE_ERROR_1(msg)                                                                                           \
    do                                                                                                                 \
    {                                                                                                                  \
        if (::TE::Log::ShouldLog("Core", "ERROR"))                                                                     \
        {                                                                                                              \
            ::TE::Log::AddMessage("Core", msg, "ERROR");                                                               \
            ::TE::Log::GetCoreLogger().log("Core", msg, "ERROR");                                                      \
        }                                                                                                              \
    } while (0)
#define TE_CORE_CRITICAL_1(msg)                                                                                        \
    do                                                                                                                 \
    {                                                                                                                  \
        if (::TE::Log::ShouldLog("Core", "CRITICAL"))                                                                  \
        {                                                                                                              \
            ::TE::Log::AddMessage("Core", msg, "CRITICAL");                                                            \
            ::TE::Log::GetCoreLogger().log("Core", msg, "CRITICAL");                                                   \
        }                                                                                                              \
    } while (0)

// Support variadic (msg + args) - simplified to just stream them
#define TE_CORE_INFO(...)                                                                                              \
    do                                                                                                                 \
    {                                                                                                                  \
        if (::TE::Log::ShouldLog("Core", "INFO"))                                                                      \
        {                                                                                                              \
            std::string msg = LogFormat(__VA_ARGS__);                                                                  \
            ::TE::Log::AddMessage("Core", msg, "INFO");                                                                \
            ::TE::Log::GetCoreLogger().log("Core", msg, "INFO");                                                       \
        }                                                                                                              \
    } while (0)
#define TE_CORE_WARN(...)                                                                                              \
    do                                                                                                                 \
    {                                                                                                                  \
        if (::TE::Log::ShouldLog("Core", "WARNING"))                                                                   \
        {                                                                                                              \
            std::string msg = LogFormat(__VA_ARGS__);                                                                  \
            ::TE::Log::AddMessage("Core", msg, "WARNING");                                                             \
            ::TE::Log::GetCoreLogger().log("Core", msg, "WARNING");                                                    \
        }                                                                                                              \
    } while (0)
#define TE_CORE_ERROR(...)                                                                                             \
    do                                                                                                                 \
    {                                                                                                                  \
        if (::TE::Log::ShouldLog("Core", "ERROR"))                                                                     \
        {                                                                                                              \
            std::string msg = LogFormat(__VA_ARGS__);                                                                  \
            ::TE::Log::AddMessage("Core", msg, "ERROR");                                                               \
            ::TE::Log::GetCoreLogger().log("Core", msg, "ERROR");                                                      \
        }                                                                                                              \
    } while (0)
#define TE_CORE_DEBUG(...)                                                                                             \
    do                                                                                                                 \
    {                                                                                                                  \
        if (::TE::Log::ShouldLog("Core", "DEBUG"))                                                                     \
        {                                                                                                              \
            std::string msg = LogFormat(__VA_ARGS__);                                                                  \
            ::TE::Log::AddMessage("Core", msg, "DEBUG");                                                               \
            ::TE::Log::GetCoreLogger().log("Core", msg, "DEBUG");                                                      \
        }                                                                                                              \
    } while (0)
#define TE_INPUT_DEBUG(...)                                                                                            \
    do                                                                                                                 \
    {                                                                                                                  \
        if (::TE::Log::ShouldLog("Input", "DEBUG"))                                                                    \
        {                                                                                                              \
            std::string msg = LogFormat(__VA_ARGS__);                                                                  \
            ::TE::Log::AddMessage("Input", msg, "DEBUG");                                                              \
            ::TE::Log::GetCoreLogger().log("Input", msg, "DEBUG");                                                     \
        }                                                                                                              \
    } while (0)
#define TE_CORE_CRITICAL(...)                                                                                          \
    do                                                                                                                 \
    {                                                                                                                  \
        if (::TE::Log::ShouldLog("Core", "CRITICAL"))                                                                  \
        {                                                                                                              \
            std::string msg = LogFormat(__VA_ARGS__);                                                                  \
            ::TE::Log::AddMessage("Core", msg, "CRITICAL");                                                            \
            ::TE::Log::GetCoreLogger().log("Core", msg, "CRITICAL");                                                   \
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
        if (::TE::Log::ShouldLog("Client", "INFO"))                                                                    \
        {                                                                                                              \
            std::string msg = LogFormat(__VA_ARGS__);                                                                  \
            ::TE::Log::AddMessage("Client", msg, "INFO");                                                              \
            ::TE::Log::GetClientLogger().log("Client", msg, "INFO");                                                   \
        }                                                                                                              \
    } while (0)
#define TE_CLIENT_WARN(...)                                                                                            \
    do                                                                                                                 \
    {                                                                                                                  \
        if (::TE::Log::ShouldLog("Client", "WARNING"))                                                                 \
        {                                                                                                              \
            std::string msg = LogFormat(__VA_ARGS__);                                                                  \
            ::TE::Log::AddMessage("Client", msg, "WARNING");                                                           \
            ::TE::Log::GetClientLogger().log("Client", msg, "WARNING");                                                \
        }                                                                                                              \
    } while (0)
#define TE_CLIENT_ERROR(...)                                                                                           \
    do                                                                                                                 \
    {                                                                                                                  \
        if (::TE::Log::ShouldLog("Client", "ERROR"))                                                                   \
        {                                                                                                              \
            std::string msg = LogFormat(__VA_ARGS__);                                                                  \
            ::TE::Log::AddMessage("Client", msg, "ERROR");                                                             \
            ::TE::Log::GetClientLogger().log("Client", msg, "ERROR");                                                  \
        }                                                                                                              \
    } while (0)
#define TE_CLIENT_DEBUG(...)                                                                                           \
    do                                                                                                                 \
    {                                                                                                                  \
        if (::TE::Log::ShouldLog("Client", "DEBUG"))                                                                   \
        {                                                                                                              \
            std::string msg = LogFormat(__VA_ARGS__);                                                                  \
            ::TE::Log::AddMessage("Client", msg, "DEBUG");                                                             \
            ::TE::Log::GetClientLogger().log("Client", msg, "DEBUG");                                                  \
        }                                                                                                              \
    } while (0)
#define TE_CLIENT_CRITICAL(...)                                                                                        \
    do                                                                                                                 \
    {                                                                                                                  \
        if (::TE::Log::ShouldLog("Client", "CRITICAL"))                                                                \
        {                                                                                                              \
            std::string msg = LogFormat(__VA_ARGS__);                                                                  \
            ::TE::Log::AddMessage("Client", msg, "CRITICAL");                                                          \
            ::TE::Log::GetClientLogger().log("Client", msg, "CRITICAL");                                               \
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
        if (::TE::Log::ShouldLog("Core", "ERROR"))                                                                     \
        {                                                                                                              \
            ::TE::Log::AddMessage("Core", msg, "ERROR");                                                               \
            ::TE::Log::GetCoreLogger().log("Core", msg, "ERROR");                                                      \
        }                                                                                                              \
    } while (0)
#define TE_CORE_CRITICAL_1(msg)                                                                                        \
    do                                                                                                                 \
    {                                                                                                                  \
        if (::TE::Log::ShouldLog("Core", "CRITICAL"))                                                                  \
        {                                                                                                              \
            ::TE::Log::AddMessage("Core", msg, "CRITICAL");                                                            \
            ::TE::Log::GetCoreLogger().log("Core", msg, "CRITICAL");                                                   \
        }                                                                                                              \
    } while (0)

#define TE_CORE_INFO(...)
#define TE_CORE_WARN(...)
#define TE_CORE_ERROR(...)                                                                                             \
    do                                                                                                                 \
    {                                                                                                                  \
        if (::TE::Log::ShouldLog("Core", "ERROR"))                                                                     \
        {                                                                                                              \
            std::string msg = LogFormat(__VA_ARGS__);                                                                  \
            ::TE::Log::AddMessage("Core", msg, "ERROR");                                                               \
            ::TE::Log::GetCoreLogger().log("Core", msg, "ERROR");                                                      \
        }                                                                                                              \
    } while (0)
#define TE_CORE_DEBUG(...)
#define TE_INPUT_DEBUG(...)
#define TE_CORE_CRITICAL(...)                                                                                          \
    do                                                                                                                 \
    {                                                                                                                  \
        if (::TE::Log::ShouldLog("Core", "CRITICAL"))                                                                  \
        {                                                                                                              \
            std::string msg = LogFormat(__VA_ARGS__);                                                                  \
            ::TE::Log::AddMessage("Core", msg, "CRITICAL");                                                            \
            ::TE::Log::GetCoreLogger().log("Core", msg, "CRITICAL");                                                   \
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
        if (::TE::Log::ShouldLog("Client", "ERROR"))                                                                   \
        {                                                                                                              \
            std::string msg = LogFormat(__VA_ARGS__);                                                                  \
            ::TE::Log::AddMessage("Client", msg, "ERROR");                                                             \
            ::TE::Log::GetClientLogger().log("Client", msg, "ERROR");                                                  \
        }                                                                                                              \
    } while (0)
#define TE_CLIENT_DEBUG(...)
#define TE_CLIENT_CRITICAL(...)                                                                                        \
    do                                                                                                                 \
    {                                                                                                                  \
        if (::TE::Log::ShouldLog("Client", "CRITICAL"))                                                                \
        {                                                                                                              \
            std::string msg = LogFormat(__VA_ARGS__);                                                                  \
            ::TE::Log::AddMessage("Client", msg, "CRITICAL");                                                          \
            ::TE::Log::GetClientLogger().log("Client", msg, "CRITICAL");                                               \
        }                                                                                                              \
    } while (0)
#define TE_CLIENT_ASSERT(x, msg)                                                                                       \
    if (!(x))                                                                                                          \
    {                                                                                                                  \
        TE_CLIENT_CRITICAL(msg);                                                                                       \
        __debugbreak();                                                                                                \
    }
#endif
