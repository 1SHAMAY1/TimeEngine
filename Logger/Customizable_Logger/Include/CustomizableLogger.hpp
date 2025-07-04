#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <fstream>
#include <mutex> 

/**
 * Customizable Logger System
 * - Register custom log levels with colors
 * - Log with custom categories and level
 * - Filter logs by level/category
 * - Console + optional JSON file output
 */
class  CustomizableLogger {
public:
    CustomizableLogger(bool toFile = false, const std::string& fileName = "log.json");
    ~CustomizableLogger();

    void registerLevel(const std::string& levelName, const std::string& ansiColor);
    void setFilterLevels(const std::vector<std::string>& levels);
    void setFilterCategories(const std::vector<std::string>& categories);

    void log(const std::string& category,
             const std::string& message,
             const std::string& level = "INFO");

    void log(const std::string& category,
             const std::string& message,
             const std::string& level,
             const char* file,
             int line);

private:
    std::unordered_map<std::string, std::string> logLevelColors;
    bool fileOutput;
    std::ofstream logFile;
    std::vector<std::string> filterLevels;
    std::vector<std::string> filterCategories;

    std::mutex logMutex;  // 🛡️ Mutex for thread safety

    std::string getColor(const std::string& level);
    std::string getResetCode();
    std::string getTimestamp();
    bool passesFilter(const std::string& item, const std::vector<std::string>& filters);
    std::string escapeJson(const std::string& raw);  // 🛡️ Escape " and \ for JSON safety
    std::string toJsonLine(const std::string& ts, const std::string& level,
                           const std::string& category, const std::string& message);

    // ----------- Enhanced Logging Macros (with file/line) -----------

#define LOG_INFO(logger, category, message) \
    logger.log(category, message, "INFO", __FILE__, __LINE__)

#define LOG_WARNING(logger, category, message) \
    logger.log(category, message, "WARNING", __FILE__, __LINE__)

#define LOG_ERROR(logger, category, message) \
    logger.log(category, message, "ERROR", __FILE__, __LINE__)

#define LOG_DEBUG(logger, category, message) \
    logger.log(category, message, "DEBUG", __FILE__, __LINE__)

#define LOG_CRITICAL(logger, category, message) \
    logger.log(category, message, "CRITICAL", __FILE__, __LINE__)

#define LOG_CUSTOM(logger, category, message, level) \
    logger.log(category, message, level, __FILE__, __LINE__)
};
