#pragma once
#include "Log.h"

extern Scope<Application> CreateApplication(int argc, char **argv);

#ifdef TE_PLATFORM_WINDOWS
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <shobjidl.h>
#include <windows.h>

inline LONG WINAPI TimeEngineCrashHandler(EXCEPTION_POINTERS *ep)
{
    if (!ep || !ep->ExceptionRecord)
        return EXCEPTION_CONTINUE_SEARCH;

    DWORD code = ep->ExceptionRecord->ExceptionCode;
    PVOID addr = ep->ExceptionRecord->ExceptionAddress;

    const char *codeStr = "UNKNOWN_EXCEPTION";
    switch (code)
    {
    case EXCEPTION_ACCESS_VIOLATION:
        codeStr = "EXCEPTION_ACCESS_VIOLATION";
        break;
    case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
        codeStr = "EXCEPTION_ARRAY_BOUNDS_EXCEEDED";
        break;
    case EXCEPTION_DATATYPE_MISALIGNMENT:
        codeStr = "EXCEPTION_DATATYPE_MISALIGNMENT";
        break;
    case EXCEPTION_FLT_DIVIDE_BY_ZERO:
        codeStr = "EXCEPTION_FLT_DIVIDE_BY_ZERO";
        break;
    case EXCEPTION_ILLEGAL_INSTRUCTION:
        codeStr = "EXCEPTION_ILLEGAL_INSTRUCTION";
        break;
    case EXCEPTION_IN_PAGE_ERROR:
        codeStr = "EXCEPTION_IN_PAGE_ERROR";
        break;
    case EXCEPTION_INT_DIVIDE_BY_ZERO:
        codeStr = "EXCEPTION_INT_DIVIDE_BY_ZERO";
        break;
    case EXCEPTION_STACK_OVERFLOW:
        codeStr = "EXCEPTION_STACK_OVERFLOW";
        break;
    }

    TE_CORE_CRITICAL("=================================================");
    TE_CORE_CRITICAL("CRASH DETECTED: {0} (Code: 0x{1:X})", codeStr, code);
    TE_CORE_CRITICAL("Faulting Address: 0x{0:X}", (uintptr_t)addr);
    TE_CORE_CRITICAL("=================================================");

    return EXCEPTION_EXECUTE_HANDLER;
}
#endif

#include "HeadlessTestRunner.hpp"

inline int RunEngine(int argc, char **argv)
{
#ifdef TE_PLATFORM_WINDOWS
    SetUnhandledExceptionFilter(TimeEngineCrashHandler);
#endif
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
    return HeadlessTestRunner::GetLastExitCode();
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
