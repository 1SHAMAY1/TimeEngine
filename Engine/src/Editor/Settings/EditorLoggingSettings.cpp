#include "Editor/Settings/EditorLoggingSettings.hpp"
#include "Core/PreRequisites.h"
#include "Core/Settings/GeneralEngineSettings.hpp"
#include "Layers/EditorLayer.hpp"
#include "Utils/TimeGUI.hpp"

TE_REGISTER_EDITOR_SETTINGS(EditorLoggingSettings);

void EditorLoggingSettings::OnDrawSettingsUI(Ref<EditorLayer> editor)
{
    auto &settings = GeneralEngineSettings::Get();

    TimeGUI::TextColored(TEColor(0.2f, 0.7f, 1.0f, 1.0f), "Output Channels");
    TimeGUI::Separator();

    bool logToConsole = settings.IsLogToConsoleEnabled();
    if (TimeGUI::Checkbox("Output to Terminal / Console", &logToConsole))
    {
        settings.SetLogToConsole(logToConsole);
    }

    bool logToFile = settings.IsLogToFileEnabled();
    if (TimeGUI::Checkbox("Write Session Log to File", &logToFile))
    {
        settings.SetLogToFile(logToFile);
    }

    bool logTimestamps = settings.IsLogTimestampEnabled();
    if (TimeGUI::Checkbox("Include Millisecond Timestamps", &logTimestamps))
    {
        settings.SetLogTimestamp(logTimestamps);
    }

    TimeGUI::Spacing();
    TimeGUI::TextColored(TEColor(0.2f, 0.7f, 1.0f, 1.0f), "Verbosity & Thresholds");
    TimeGUI::Separator();

    const char *logLevels[] = {"DEBUG", "INFO", "WARNING", "ERROR", "CRITICAL"};
    int currentLevelIdx = 1;
    TEString curLevel = settings.GetLogLevel();
    for (int i = 0; i < 5; ++i)
    {
        if (curLevel == logLevels[i])
        {
            currentLevelIdx = i;
            break;
        }
    }

    if (TimeGUI::Combo("Minimum Log Level", &currentLevelIdx, logLevels, 5))
    {
        settings.SetLogLevel(logLevels[currentLevelIdx]);
    }

    TimeGUI::Spacing();
    TimeGUI::TextColored(TEColor(0.2f, 0.7f, 1.0f, 1.0f), "Active Subsystem Channels");
    TimeGUI::Separator();

    auto &categories = settings.GetLogCategories();
    for (auto it = categories.begin(); it != categories.end(); ++it)
    {
        bool enabled = it->second;
        TEString label = it->first + " Channel";
        if (TimeGUI::Checkbox(label, &enabled))
        {
            settings.SetLogCategory(it->first, enabled);
        }
    }

    TimeGUI::Spacing();
    TimeGUI::Separator();

    if (TimeGUI::Button("Reset Logging Defaults", 200.0f, 26.0f))
    {
        settings.SetLogToConsole(true);
        settings.SetLogToFile(true);
        settings.SetLogTimestamp(true);
        settings.SetLogLevel("INFO");
        settings.SetLogCategory("Core", true);
        settings.SetLogCategory("Renderer", true);
        settings.SetLogCategory("Input", true);
        settings.SetLogCategory("Audio", true);
        settings.SetLogCategory("Physics", true);
    }
}
