#pragma once
#include "PreRequisites.h"

namespace TE
{

    class TE_API  Application
    {
    public:
        Application();
        virtual ~Application();
        void Run();
    };

    Application* CreateApplication();
}

