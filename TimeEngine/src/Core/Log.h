#pragma once

#include "PreRequisites.h"
#include "spdlog/spdlog.h"
#include <memory>

namespace TimeEngine
{
    class TIMEENGINE_API Log
    {
    public:
        static void Init();
        inline static std::shared_ptr<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; }
        inline static std::shared_ptr<spdlog::logger>& GetClientLogger() { return s_ClientLogger; }
private:
        static std::shared_ptr<spdlog::logger> s_CoreLogger;
        static std::shared_ptr<spdlog::logger> s_ClientLogger;
        
    };
}
#define TE_CORE_INFO(...) TimeEngine     ::Log::GetCoreLogger()->info(__VA_ARGS__)
#define TE_CORE_TRACE(...) TimeEngine    ::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define TE_CORE_ERROR(...) TimeEngine    ::Log::GetCoreLogger()->error(__VA_ARGS__)
#define TE_CORE_WARN(...) TimeEngine     ::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define TE_CORE_FATAL(...) TimeEngine    ::Log::GetCoreLogger()->fatal(__VA_ARGS__)

#define TE_CLIENT_INFO(...) TimeEngine     ::Log::GetClientLogger()->info(__VA_ARGS__)
#define TE_CLIENT_TRACE(...) TimeEngine    ::Log::GetClientLogger()->trace(__VA_ARGS__)
#define TE_CLIENT_ERROR(...) TimeEngine    ::Log::GetClientLogger()->error(__VA_ARGS__)
#define TE_CLIENT_WARN(...) TimeEngine     ::Log::GetClientLogger()->warn(__VA_ARGS__)
#define TE_CLIENT_FATAL(...) TimeEngine    ::Log::GetClientLogger()->fatal(__VA_ARGS__)