#pragma once
#include "PreRequisites.h"

namespace TimeEngine
{

    class TIMEENGINE_API  Application
    {
    public:
        Application();
        virtual ~Application();
        void Run();
    };

    Application* CreateApplication();
}

