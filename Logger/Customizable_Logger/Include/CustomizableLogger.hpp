#pragma once


#include <string>
#include <unordered_map>
#include <vector>
#include <fstream>
#include "LoggerAPI.hpp"


/**
 * Customizable Logger System
 * - Register custom log levels with colors
 * - Log with custom categories and level
 * - Filter logs by level/category
 * - Console + optional JSON file output
 */

class LOGGER_API  CustomizableLogger {
public:
    /**
     * Constructor
     * @param toFile enable JSON file output
     * @param fileName name of the log file
     */
    CustomizableLogger(bool toFile = false, const std::string& fileName = "log.json");

    /**
     * Destructor
     */
    ~CustomizableLogger();

    /**
     * Register a new log level with a color
     * @param levelName e.g. "DEBUG", "GAMEPLAY/AI"
     * @param ansiColor e.g. "\033[32m"
     */
    void registerLevel(const std::string& levelName, const std::string& ansiColor);

    /**
     * Filter logs by allowed levels (prefix match supported)
     */
    void setFilterLevels(const std::vector<std::string>& levels);

    /**
     * Filter logs by allowed categories (prefix match supported)
     */
    void setFilterCategories(const std::vector<std::string>& categories);

    /**
     * Log a message
     * @param category e.g. "SYSTEM", "UI/CLICK"
     * @param message the log message
     * @param level log level (default "INFO")
     */
    void log(const std::string& category, const std::string& message, const std::string& level = "INFO");

private:
    std::unordered_map<std::string, std::string> logLevelColors;
    bool fileOutput;
    std::ofstream logFile;
    std::vector<std::string> filterLevels;
    std::vector<std::string> filterCategories;

    std::string getColor(const std::string& level);
    std::string getResetCode();
    std::string getTimestamp();
    bool passesFilter(const std::string& item, const std::vector<std::string>& filters);
    std::string toJsonLine(const std::string& ts, const std::string& level,
                           const std::string& category, const std::string& message);


    // ----------- Logging Macros -----------

	#define LOG_INFO(logger, category, message) \
	logger.log(category, message, "INFO")

	#define LOG_WARNING(logger, category, message) \
	logger.log(category, message, "WARNING")

	#define LOG_ERROR(logger, category, message) \
	logger.log(category, message, "ERROR")

	#define LOG_DEBUG(logger, category, message) \
	logger.log(category, message, "DEBUG")

	#define LOG_CRITICAL(logger, category, message) \
	logger.log(category, message, "CRITICAL")

    // For custom log level (e.g. "GAMEPLAY/AI")
	#define LOG_CUSTOM(logger, category, message, level) \
	logger.log(category, message, level)

};
