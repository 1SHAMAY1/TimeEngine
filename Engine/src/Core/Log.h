#pragma once

#include "PreRequisites.h"
#include "CustomizableLogger.hpp"
#include <memory>
#include <string>

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

// ======= Logging Macros using your logger =======

#define TE_CORE_INFO(msg)      TE::Log::GetCoreLogger().log("Core", msg, "INFO")
#define TE_CORE_WARN(msg)      TE::Log::GetCoreLogger().log("Core", msg, "WARNING")
#define TE_CORE_ERROR(msg)     TE::Log::GetCoreLogger().log("Core", msg, "ERROR")
#define TE_CORE_DEBUG(msg)     TE::Log::GetCoreLogger().log("Core", msg, "DEBUG")
#define TE_CORE_CRITICAL(msg)  TE::Log::GetCoreLogger().log("Core", msg, "CRITICAL")

#define TE_CLIENT_INFO(msg)     TE::Log::GetClientLogger().log("Client", msg, "INFO")
#define TE_CLIENT_WARN(msg)     TE::Log::GetClientLogger().log("Client", msg, "WARNING")
#define TE_CLIENT_ERROR(msg)    TE::Log::GetClientLogger().log("Client", msg, "ERROR")
#define TE_CLIENT_DEBUG(msg)    TE::Log::GetClientLogger().log("Client", msg, "DEBUG")
#define TE_CLIENT_CRITICAL(msg) TE::Log::GetClientLogger().log("Client", msg, "CRITICAL")
