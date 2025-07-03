#pragma once

#include "PreRequisites.h"
#include "Window/IWindow.hpp"
#include <memory>

namespace TE
{
    class TE_API Application
    {
    public:
        Application();
        virtual ~Application();

        void Run();

    private:
        std::unique_ptr<IWindow> m_Window;  // Window abstraction pointer
        bool m_Running;
    };

    // To be defined by the client (e.g., Sandbox app)
    Application* CreateApplication();
}
