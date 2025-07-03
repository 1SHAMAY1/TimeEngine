#include "Log.h"

namespace TE
{
    std::unique_ptr<CustomizableLogger> Log::s_CoreLogger;
    std::unique_ptr<CustomizableLogger> Log::s_ClientLogger;

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
}
