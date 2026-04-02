#pragma once
#include "Log.h"

#ifdef TE_PLATFORM_WINDOWS

extern TE::Application *TE::CreateApplication(int argc, char **argv);

inline int RunEngine(int argc, char **argv)
{
    try
    {
        TE::Log::Init();
        TE_CORE_INFO("Log Initialized!");
        TE_CLIENT_INFO("Welcome to Time Engine.");

        auto project = TE::CreateApplication(argc, argv);
        if (project)
        {
            project->Run();
            delete project;
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

#ifdef TE_PACKAGED
#include <windows.h>
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    return RunEngine(__argc, __argv);
}
#else
int main(int argc, char **argv)
{
    return RunEngine(argc, argv);
}
#endif

#endif