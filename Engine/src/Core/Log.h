#pragma once

#include "PreRequisites.h"
#include "CustomizableLogger.hpp"

namespace TE
{
    class TE_API Log
    {
    public:
        static void Init(bool logToFile = true, const std::string& file = "TimeEngineLog.json");

        inline static CustomizableLogger& GetCoreLogger()   { return *s_CoreLogger; }
        inline static CustomizableLogger& GetClientLogger() { return *s_ClientLogger; }

    private:
        // DO NOT export STL members; no TE_API here to avoid warning C4251
        static std::unique_ptr<CustomizableLogger> s_CoreLogger;
        static std::unique_ptr<CustomizableLogger> s_ClientLogger;
    };
}

// ======= Logging Macros using your logger =======

// -------- Core Logger Macros with File/Line --------
#define TE_CORE_INFO(msg)      ::TE::Log::GetCoreLogger().log("Core", msg, "INFO",     __FILE__, __LINE__)
#define TE_CORE_WARN(msg)      ::TE::Log::GetCoreLogger().log("Core", msg, "WARNING",  __FILE__, __LINE__)
#define TE_CORE_ERROR(msg)     ::TE::Log::GetCoreLogger().log("Core", msg, "ERROR",    __FILE__, __LINE__)
#define TE_CORE_DEBUG(msg)     ::TE::Log::GetCoreLogger().log("Core", msg, "DEBUG",    __FILE__, __LINE__)
#define TE_CORE_CRITICAL(msg)  ::TE::Log::GetCoreLogger().log("Core", msg, "CRITICAL", __FILE__, __LINE__)

// -------- Client Logger Macros with File/Line --------
#define TE_CLIENT_INFO(msg)     ::TE::Log::GetClientLogger().log("Client", msg, "INFO",     __FILE__, __LINE__)
#define TE_CLIENT_WARN(msg)     ::TE::Log::GetClientLogger().log("Client", msg, "WARNING",  __FILE__, __LINE__)
#define TE_CLIENT_ERROR(msg)    ::TE::Log::GetClientLogger().log("Client", msg, "ERROR",    __FILE__, __LINE__)
#define TE_CLIENT_DEBUG(msg)    ::TE::Log::GetClientLogger().log("Client", msg, "DEBUG",    __FILE__, __LINE__)
#define TE_CLIENT_CRITICAL(msg) ::TE::Log::GetClientLogger().log("Client", msg, "CRITICAL", __FILE__, __LINE__)
