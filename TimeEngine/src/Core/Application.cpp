#include "Application.h"
#include "Events/ApplicationEvent.h"
#include "Log.h"

namespace TimeEngine
{
    Application::Application()
    {
    }

    Application::~Application()
    {
    }

    void Application::Run()
    {
        WindowResizeEvent e(1280,720);
        TE_CLIENT_TRACE(e)
        while (true);
    }
}
