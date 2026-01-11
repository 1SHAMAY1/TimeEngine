#pragma once
#include "Log.h"

#ifdef TE_PLATFORM_WINDOWS

extern TE::Application* TE::CreateApplication(int argc, char** argv);

int main(int argc, char** argv)
{
    try {
        TE::Log::Init();
        TE_CORE_INFO("Log Initialized!");
        TE_CLIENT_INFO("Welcome to Time Engine.");

        auto project = TE::CreateApplication(argc, argv);
        project->Run();
        delete project;
    }
    catch (const std::exception& e)
    {
        TE_CORE_CRITICAL("Unhandled Exception: {0}", e.what());
        // Attempt to log to file explicitly if logging failed? 
        // Assuming Log is initialized enough.
    }
    catch (...)
    {
        TE_CORE_CRITICAL("Unknown Unhandled Exception!");
    }
}
#endif