#pragma once
#include "Log.h"

#ifdef TE_PLATFORM_WINDOWS

extern TE::Application* TE::CreateApplication();

int main(int argc, char** argv)
{
    auto project = TE::CreateApplication();
    TE::Log::Init();
    TE_CORE_INFO("Log Initialized!");
    TE_CLIENT_INFO("Welcome to Time Engine.");
    project->Run();
    delete project;
}
#endif