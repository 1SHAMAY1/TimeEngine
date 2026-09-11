#pragma once
#include "Log.h"

extern Scope<Application> CreateApplication(int argc, char **argv);

#ifdef TE_PLATFORM_WINDOWS
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <shobjidl.h>
#include <windows.h>
#endif

inline int RunEngine(int argc, char **argv)
{
    try
    {
        bool silentMode = false;
        TEString exePath = PlatformUtils::GetExecutablePath();
        if (!exePath.IsEmpty())
        {
            TEString appDir = exePath.GetParentPath();
            TEString iniPath = appDir / "Config" / "DefaultEngine.ini";
            if (TEFileSystem::Exists(iniPath))
            {
                TEFileSystem::ForEachLine(iniPath,
                                          [&](const TEString &line) -> bool
                                          {
                                              if (line.find("Verbosity=Error") != TEString::npos ||
                                                  line.find("Configuration=Shipping") != TEString::npos)
                                              {
                                                  silentMode = true;
                                              }
                                              return true;
                                          });
            }
        }

#ifdef TE_PLATFORM_WINDOWS
        if (silentMode)
        {
            ::FreeConsole();
        }
#endif

        Log::Init(true, "TimeEngineLog.json", silentMode);
        if (!silentMode)
        {
            TE_CORE_INFO("Log Initialized!");
            TE_CLIENT_INFO("Welcome to Time Engine.");
        }

#ifdef TE_PLATFORM_WINDOWS
        SetCurrentProcessExplicitAppUserModelID(L"TimeEngine.TimeEditor");
#endif

        auto project = CreateApplication(argc, argv);
        if (project)
        {
            project->Run();
            project.reset();
        }
    }
    catch (const std::exception &e)
    {
        TE_CORE_CRITICAL("Unhandled Exception: {0}", e.what());
    }
    catch (...)
    {
        TE_CORE_CRITICAL("Unknown Unhandled Exception!");
    }
    return 0;
}

#ifdef TE_PLATFORM_WINDOWS
#ifdef TE_PACKAGED
#include <windows.h>
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    return RunEngine(__argc, __argv);
}
#else
int main(int argc, char **argv) { return RunEngine(argc, argv); }
#endif
#else
int main(int argc, char **argv) { return RunEngine(argc, argv); }
#endif
