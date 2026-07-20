#include "Log.h"
#include "Core/EngineSettings.hpp"
#include <mutex>
#include <chrono>
#include <iomanip>

namespace TE
{
    std::unique_ptr<CustomizableLogger> Log::s_CoreLogger;
    std::unique_ptr<CustomizableLogger> Log::s_ClientLogger;

    static std::vector<LogMessage> s_MessageBuffer;
    static std::mutex s_LogMutex;
    static constexpr size_t MAX_LOG_MESSAGES = 1000;

    void Log::Init(bool logToFile, const std::string& file)
    {
        s_CoreLogger = std::make_unique<CustomizableLogger>(logToFile, "Core_" + file);
        s_ClientLogger = std::make_unique<CustomizableLogger>(logToFile, "Client_" + file);

        // Core log level colors
        s_CoreLogger->registerLevel("INFO",     "\033[32m");  // Green
        s_CoreLogger->registerLevel("WARNING",  "\033[33m");  // Yellow
        s_CoreLogger->registerLevel("ERROR",    "\033[31m");  // Red
        s_CoreLogger->registerLevel("DEBUG",    "\033[34m");  // Blue
        s_CoreLogger->registerLevel("CRITICAL", "\033[41m");  // Red background

        // Client log level colors
        s_ClientLogger->registerLevel("INFO",     "\033[36m");  // Cyan
        s_ClientLogger->registerLevel("WARNING",  "\033[35m");  // Magenta
        s_ClientLogger->registerLevel("ERROR",    "\033[91m");  // Bright Red
        s_ClientLogger->registerLevel("DEBUG",    "\033[94m");  // Bright Blue
        s_ClientLogger->registerLevel("CRITICAL", "\033[41m");
    }

    void Log::AddMessage(const std::string &category, const std::string &message, const std::string &level)
    {
        std::lock_guard<std::mutex> lock(s_LogMutex);

        auto now = std::chrono::system_clock::now();
        auto in_time_t = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

        std::stringstream ss;
        struct tm buf;
#ifdef _WIN32
        localtime_s(&buf, &in_time_t);
#else
        localtime_r(&in_time_t, &buf);
#endif
        ss << std::put_time(&buf, "%Y-%m-%d %H:%M:%S")
           << '.' << std::setfill('0') << std::setw(3) << ms.count();

        LogMessage msg;
        msg.Timestamp = ss.str();
        msg.Category = category;
        msg.Message = message;
        msg.Level = level;

        s_MessageBuffer.push_back(std::move(msg));
        if (s_MessageBuffer.size() > MAX_LOG_MESSAGES)
        {
            s_MessageBuffer.erase(s_MessageBuffer.begin());
        }
    }

    std::vector<LogMessage> Log::GetMessageBuffer()
    {
        std::lock_guard<std::mutex> lock(s_LogMutex);
        return s_MessageBuffer;
    }

    void Log::ClearMessageBuffer()
    {
        std::lock_guard<std::mutex> lock(s_LogMutex);
        s_MessageBuffer.clear();
    }

    bool Log::ShouldLog(const std::string &category, const std::string &level)
    {
        thread_local bool insideShouldLog = false;
        if (insideShouldLog)
            return true;

        insideShouldLog = true;
        auto &settings = EngineSettings::Get();
        if (!settings.IsLogCategoryEnabled(category))
        {
            insideShouldLog = false;
            return false;
        }

        auto LevelToInt = [](const std::string &lvl) -> int {
            if (lvl == "DEBUG") return 0;
            if (lvl == "INFO") return 1;
            if (lvl == "WARNING") return 2;
            if (lvl == "ERROR") return 3;
            if (lvl == "CRITICAL") return 4;
            return 1;
        };

        if (LevelToInt(level) < LevelToInt(settings.GetLogLevel()))
        {
            insideShouldLog = false;
            return false;
        }

        insideShouldLog = false;
        return true;
    }
}
