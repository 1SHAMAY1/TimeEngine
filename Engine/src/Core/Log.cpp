#include "Log.h"
#include "Core/PreRequisites.h"
#include "Core/Settings/GeneralEngineSettings.hpp"
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <mutex>

#ifdef _WIN32
#include <io.h>
#include <windows.h>
#else
#include <unistd.h>
#endif

TEScope<CustomizableLogger> Log::s_CoreLogger;
TEScope<CustomizableLogger> Log::s_ClientLogger;

static TEArray<LogMessage> s_MessageBuffer;
static std::mutex s_LogMutex;
static constexpr size_t MAX_LOG_MESSAGES = 1000;

static bool s_Initialized = false;

struct TermCaps
{
    bool isTTY = false;
    bool ansi = false;
    bool truecolor = false;
};

static TermCaps DetectTerminal()
{
    TermCaps caps;
#ifdef _WIN32
    caps.isTTY = (_isatty(_fileno(stdout)) != 0);
    if (caps.isTTY)
    {
        HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
        DWORD mode = 0;
        if (hOut != INVALID_HANDLE_VALUE && GetConsoleMode(hOut, &mode))
        {
            if (SetConsoleMode(hOut, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING))
                caps.ansi = true;
        }
        SetConsoleOutputCP(CP_UTF8);
        SetConsoleCP(CP_UTF8);
    }
    caps.truecolor = caps.ansi;
#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4996)
#endif
    const char *noColorEnv = std::getenv("NO_COLOR");
    if (noColorEnv && noColorEnv[0] != '\0')
    {
        caps.ansi = false;
        caps.truecolor = false;
    }
#if defined(_MSC_VER)
#pragma warning(pop)
#endif
#else
    caps.isTTY = (isatty(fileno(stdout)) != 0);
    const char *term = std::getenv("TERM");
    const char *colorterm = std::getenv("COLORTERM");
    caps.ansi = caps.isTTY && term && TEString(term) != "dumb";
    caps.truecolor = caps.ansi && colorterm && (TEString(colorterm) == "truecolor" || TEString(colorterm) == "24bit");
    if (std::getenv("NO_COLOR"))
    {
        caps.ansi = false;
        caps.truecolor = false;
    }
#endif
    return caps;
}

struct RGB
{
    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;
    bool empty = true;
};

static void PrintBanner(const TermCaps &caps)
{
    if (!caps.isTTY || !caps.ansi)
    {
        // Clean Plain-Text Fallback for redirected file output / non-ANSI terminals
        std::printf("\n========================================================================\n");
        std::printf("                     TIME ENGINE - Welcome to TimeEngine                \n");
        std::printf("========================================================================\n\n");
        std::fflush(stdout);
        return;
    }

    // 14x14 RGB Bitmap of TimeEngineIcon.png
    RGB C{0, 235, 255, false};   // Cyan outer ring
    RGB c{0, 160, 200, false};   // Dark cyan inner ring
    RGB R{255, 60, 60, false};   // Red 3 o'clock arrow hand
    RGB W{245, 245, 245, false}; // White vertical stem / hub
    RGB _{0, 0, 0, true};        // Empty / transparent

    TEArray<TEArray<RGB>> bitmap = {
        {_, _, C, C, C, C, C, C, C, C, C, _, _, _}, {_, C, _, _, _, _, _, _, _, _, _, C, _, _},
        {C, _, _, c, c, c, c, c, c, _, _, C, _, _}, {C, _, c, _, _, _, _, _, _, c, _, C, _, _},
        {C, _, c, _, _, _, _, _, _, c, _, C, _, _}, {C, c, _, W, W, _, R, R, R, R, c, C, _, _},
        {C, _, c, _, _, W, _, _, _, c, _, C, _, _}, {C, _, _, c, _, W, _, c, c, _, _, C, _, _},
        {C, _, _, _, c, W, c, _, _, _, _, C, _, _}, {_, C, _, _, _, W, _, _, _, _, C, _, _, _},
        {_, _, C, C, C, W, C, C, C, C, _, _, _, _}, {_, _, _, _, _, W, _, _, _, _, _, _, _, _},
        {_, _, _, _, _, W, _, _, _, _, _, _, _, _}, {_, _, _, _, _, _, _, _, _, _, _, _, _, _}};

    // Title lines paired alongside the 7 half-block pixel rows
    const char *magenta = "\033[95m";
    const char *brightCyan = "\033[96m";
    const char *yellow = "\033[93m";
    const char *reset = "\033[0m";

    TEArray<TEString> titleLines = {TEString(magenta) + "████████╗██╗███╗   ███╗███████╗ " + brightCyan +
                                        "███████╗███╗   ██╗██████╗ ██╗███╗   ██╗███████╗" + reset,
                                    TEString(magenta) + "╚══██╔══╝██║████╗ ████║██╔════╝ " + brightCyan +
                                        "██╔════╝████╗  ██║██╔════╝ ██║████╗  ██║██╔════╝" + reset,
                                    TEString(magenta) + "   ██║   ██║██╔████╔██║█████╗   " + brightCyan +
                                        "█████╗  ██╔██╗ ██║██║  ███╗██║██╔██╗ ██║█████╗  " + reset,
                                    TEString(magenta) + "   ██║   ██║██║╚██╔╝██║██╔══╝   " + brightCyan +
                                        "██╔══╝  ██║╚██╗██║██║   ██║██║██║╚██╗██║██╔══╝  " + reset,
                                    TEString(magenta) + "   ██║   ██║██║ ╚═╝ ██║███████╗ " + brightCyan +
                                        "███████╗██║ ╚████║╚██████╔╝██║██║ ╚████║███████╗" + reset,
                                    TEString(magenta) + "   ╚═╝   ╚═╝╚═╝     ╚═╝╚══════╝ " + brightCyan +
                                        "╚══════╝╚═╝  ╚═══╝ ╚═════╝ ╚═╝╚═╝  ╚═══╝╚══════╝" + reset,
                                    TEString(yellow) + "                                 Welcome to TimeEngine" +
                                        reset};

    std::printf("\n");
    for (size_t y = 0; y < 14; y += 2)
    {
        size_t lineIdx = y / 2;
        // Print 14 half-block columns for left icon
        for (size_t x = 0; x < 14; ++x)
        {
            const RGB &top = bitmap[y][x];
            const RGB &bot = bitmap[y + 1][x];

            if (top.empty && bot.empty)
            {
                std::printf(" ");
            }
            else
            {
                if (!top.empty)
                    std::printf("\033[38;2;%d;%d;%dm", top.r, top.g, top.b);
                else
                    std::printf("\033[39m");

                if (!bot.empty)
                    std::printf("\033[48;2;%d;%d;%dm", bot.r, bot.g, bot.b);
                else
                    std::printf("\033[49m");

                std::printf("▀");
            }
        }
        std::printf("%s   ", reset);

        // Print corresponding title line
        if (lineIdx < titleLines.size())
            std::printf("%s", titleLines[lineIdx].c_str());

        std::printf("\n");
    }

    std::printf("\033[36m----------------------------------------------------------------------------------------------"
                "---------------------%s\n\n",
                reset);
    std::fflush(stdout);
}

void Log::Init(bool logToFile, const TEString &file)
{
    if (s_Initialized)
        return;
    s_Initialized = true;

    TermCaps caps = DetectTerminal();
    PrintBanner(caps);

    s_CoreLogger = CreateScope<CustomizableLogger>(logToFile, "Core_" + file);
    s_ClientLogger = CreateScope<CustomizableLogger>(logToFile, "Client_" + file);

    // Core log level colors
    s_CoreLogger->registerLevel("INFO", "\033[32m");     // Green
    s_CoreLogger->registerLevel("WARNING", "\033[33m");  // Yellow
    s_CoreLogger->registerLevel("ERROR", "\033[31m");    // Red
    s_CoreLogger->registerLevel("DEBUG", "\033[34m");    // Blue
    s_CoreLogger->registerLevel("CRITICAL", "\033[41m"); // Red background

    // Client log level colors
    s_ClientLogger->registerLevel("INFO", "\033[36m");    // Cyan
    s_ClientLogger->registerLevel("WARNING", "\033[35m"); // Magenta
    s_ClientLogger->registerLevel("ERROR", "\033[91m");   // Bright Red
    s_ClientLogger->registerLevel("DEBUG", "\033[94m");   // Bright Blue
    s_ClientLogger->registerLevel("CRITICAL", "\033[41m");
}

void Log::AddMessage(const TEString &category, const TEString &message, const TEString &level)
{
    std::lock_guard<std::mutex> lock(s_LogMutex);

    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

    struct tm buf;
#ifdef _WIN32
    localtime_s(&buf, &in_time_t);
#else
    localtime_r(&in_time_t, &buf);
#endif
    TEString timeStr;
    timeStr.Reserve(64);
    std::strftime(timeStr.Data(), 64, "%Y-%m-%d %H:%M:%S", &buf);

    TEString msStr = "." + TEString::FromInt((int)ms.count());

    LogMessage msg;
    msg.Timestamp = timeStr.c_str() + msStr;
    msg.Category = category;
    msg.Message = message;
    msg.Level = level;

    s_MessageBuffer.push_back(std::move(msg));
    if (s_MessageBuffer.size() > MAX_LOG_MESSAGES)
    {
        s_MessageBuffer.erase(s_MessageBuffer.begin());
    }
}

TEArray<LogMessage> Log::GetMessageBuffer()
{
    std::lock_guard<std::mutex> lock(s_LogMutex);
    return s_MessageBuffer;
}

void Log::ClearMessageBuffer()
{
    std::lock_guard<std::mutex> lock(s_LogMutex);
    s_MessageBuffer.clear();
}

bool Log::ShouldLog(const TEString &category, const TEString &level)
{
    thread_local bool insideShouldLog = false;
    if (insideShouldLog)
        return true;

    insideShouldLog = true;
    auto &settings = EngineSettingsRegistry::GetMutable<GeneralEngineSettings>();
    if (!settings.IsLogCategoryEnabled(category))
    {
        insideShouldLog = false;
        return false;
    }

    auto LevelToInt = [](const TEString &lvl) -> int
    {
        if (lvl == "DEBUG")
            return 0;
        if (lvl == "INFO")
            return 1;
        if (lvl == "WARNING")
            return 2;
        if (lvl == "ERROR")
            return 3;
        if (lvl == "CRITICAL")
            return 4;
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

TEVector4 Log::GetLogColor(const TEString &category, const TEString &level)
{
    // WARNING (Yellow / Gold)
    if (level == "WARN" || level == "WARNING")
    {
        return TEVector4(1.0f, 0.85f, 0.0f, 1.0f);
    }
    // ERROR & CRITICAL & FATAL (Bright Red)
    if (level == "ERROR" || level == "CRITICAL" || level == "FATAL")
    {
        return TEVector4(1.0f, 0.20f, 0.25f, 1.0f);
    }
    // DEBUG & TRACE (Light Sky Blue)
    if (level == "DEBUG" || level == "TRACE")
    {
        return TEVector4(0.40f, 0.65f, 1.0f, 1.0f);
    }
    // INFO levels: Distinguish Core (Green) vs Client / Plugins (Cyan/Blue)
    if (level == "INFO")
    {
        if (category == "Client" || category.StartsWith("Client"))
        {
            return TEVector4(0.0f, 0.70f, 1.0f, 1.0f); // Bright Terminal Cyan (#00B0FF)
        }
        return TEVector4(0.0f, 0.90f, 0.15f, 1.0f); // Bright Terminal Green (#00E676)
    }

    return TEVector4(0.85f, 0.90f, 0.95f, 1.0f);
}
