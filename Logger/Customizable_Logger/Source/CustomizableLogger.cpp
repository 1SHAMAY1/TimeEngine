#include "CustomizableLogger.hpp"
#include <iostream>
#include <chrono>
#include <iomanip>
#include <sstream>

CustomizableLogger::CustomizableLogger(bool toFile, const std::string& fileName)
    : fileOutput(toFile) {
    if (fileOutput) logFile.open(fileName, std::ios::out | std::ios::app);

    registerLevel("INFO", "\033[32m");
    registerLevel("WARNING", "\033[33m");
    registerLevel("ERROR", "\033[31m");
    registerLevel("DEBUG", "\033[34m");
    registerLevel("CRITICAL", "\033[41m");
}

CustomizableLogger::~CustomizableLogger() {
    if (logFile.is_open()) logFile.close();
}

void CustomizableLogger::registerLevel(const std::string& lvl, const std::string& color) {
    logLevelColors[lvl] = color;
}

void CustomizableLogger::setFilterLevels(const std::vector<std::string>& levels) {
    filterLevels = levels;
}
void CustomizableLogger::setFilterCategories(const std::vector<std::string>& categories) {
    filterCategories = categories;
}

std::string CustomizableLogger::getColor(const std::string& level) {
    auto it = logLevelColors.find(level);
    return it != logLevelColors.end() ? it->second : "\033[0m";
}

std::string CustomizableLogger::getResetCode() {
    return "\033[0m";
}

std::string CustomizableLogger::getTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;

    std::tm localTime;
#if defined(_MSC_VER)
    localtime_s(&localTime, &t);  // Safe version for MSVC
#else
    localTime = *std::localtime(&t);  // Safe on POSIX
#endif

    std::ostringstream oss;
    oss << std::put_time(&localTime, "%Y-%m-%d %H:%M:%S")
        << "." << std::setw(3) << std::setfill('0') << ms.count();
    return oss.str();
}


bool CustomizableLogger::passesFilter(const std::string& item,
                                      const std::vector<std::string>& filters) {
    if (filters.empty()) return true;
    for (const auto& f : filters) {
        if (item.rfind(f, 0) == 0) return true;
    }
    return false;
}

std::string CustomizableLogger::toJsonLine(const std::string& ts, const std::string& level,
                                           const std::string& cat, const std::string& msg) {
    std::ostringstream oss;
    oss << "{"
        << "\"timestamp\":\"" << ts << "\","
        << "\"level\":\"" << level << "\","
        << "\"category\":\"" << cat << "\","
        << "\"message\":\"" << msg << "\"}";
    return oss.str();
}

void CustomizableLogger::log(const std::string& category, const std::string& message,
                             const std::string& level) {
    if (!passesFilter(level, filterLevels)) return;
    if (!passesFilter(category, filterCategories)) return;

    std::string ts = getTimestamp();
    std::string color = getColor(level);
    std::string reset = getResetCode();
    std::string formatted = "[" + ts + "] [" + level + "] [" + category + "] " + message;

    std::cout << color << formatted << reset << std::endl;

    if (fileOutput && logFile.is_open()) {
        logFile << toJsonLine(ts, level, category, message) << "\n";
    }
}
