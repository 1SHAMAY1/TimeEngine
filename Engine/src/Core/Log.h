#pragma once

#include "PreRequisites.h"
#include "CustomizableLogger.hpp"
#include <string>
#include <sstream>

namespace TE
{
    class TE_API Log
    {
    public:
        static void Init(bool logToFile = true, const std::string& file = "TimeEngineLog.json");

        inline static CustomizableLogger& GetCoreLogger()   { return *s_CoreLogger; }
        inline static CustomizableLogger& GetClientLogger() { return *s_ClientLogger; }

    private:
        static std::unique_ptr<CustomizableLogger> s_CoreLogger;
        static std::unique_ptr<CustomizableLogger> s_ClientLogger;
    };
}

// Simple variadic string joiner for logs to avoid fmt dependency issues
template<typename... Args>
std::string LogFormat(Args&&... args) {
    std::ostringstream ss;
    (ss << ... << args); 
    return ss.str();
}

// Support single argument (msg only)
#define TE_CORE_INFO_1(msg)      ::TE::Log::GetCoreLogger().log("Core", msg, "INFO",     __FILE__, __LINE__)
#define TE_CORE_WARN_1(msg)      ::TE::Log::GetCoreLogger().log("Core", msg, "WARNING",  __FILE__, __LINE__)
#define TE_CORE_ERROR_1(msg)     ::TE::Log::GetCoreLogger().log("Core", msg, "ERROR",    __FILE__, __LINE__)
#define TE_CORE_CRITICAL_1(msg)  ::TE::Log::GetCoreLogger().log("Core", msg, "CRITICAL", __FILE__, __LINE__)

// Support variadic (msg + args) - simplified to just stream them
#define TE_CORE_INFO(...)      ::TE::Log::GetCoreLogger().log("Core", LogFormat(__VA_ARGS__), "INFO",     __FILE__, __LINE__)
#define TE_CORE_WARN(...)      ::TE::Log::GetCoreLogger().log("Core", LogFormat(__VA_ARGS__), "WARNING",  __FILE__, __LINE__)
#define TE_CORE_ERROR(...)     ::TE::Log::GetCoreLogger().log("Core", LogFormat(__VA_ARGS__), "ERROR",    __FILE__, __LINE__)
#define TE_CORE_DEBUG(...)     ::TE::Log::GetCoreLogger().log("Core", LogFormat(__VA_ARGS__), "DEBUG",    __FILE__, __LINE__)
#define TE_CORE_CRITICAL(...)  ::TE::Log::GetCoreLogger().log("Core", LogFormat(__VA_ARGS__), "CRITICAL", __FILE__, __LINE__)

#define TE_CORE_ASSERT(x, msg) if (!(x)) { TE_CORE_CRITICAL(msg); __debugbreak(); }

#define TE_CLIENT_INFO(...)     ::TE::Log::GetClientLogger().log("Client", LogFormat(__VA_ARGS__), "INFO",     __FILE__, __LINE__)
#define TE_CLIENT_WARN(...)     ::TE::Log::GetClientLogger().log("Client", LogFormat(__VA_ARGS__), "WARNING",  __FILE__, __LINE__)
#define TE_CLIENT_ERROR(...)    ::TE::Log::GetClientLogger().log("Client", LogFormat(__VA_ARGS__), "ERROR",    __FILE__, __LINE__)
#define TE_CLIENT_DEBUG(...)    ::TE::Log::GetClientLogger().log("Client", LogFormat(__VA_ARGS__), "DEBUG",    __FILE__, __LINE__)
#define TE_CLIENT_CRITICAL(...) ::TE::Log::GetClientLogger().log("Client", LogFormat(__VA_ARGS__), "CRITICAL", __FILE__, __LINE__)
#define TE_CLIENT_ASSERT(x, msg) if (!(x)) { TE_CLIENT_CRITICAL(msg); __debugbreak(); }
