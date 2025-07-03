#include "Application.h"
#include "Events/ApplicationEvent.h"
#include "Log.h"

namespace TE
{
    Application::Application()
    {
        // Initialize the logging system here (once)
        TE::Log::Init(true, "TimeEngineLog.json");

        TE_CORE_INFO("Application Constructor called.");
    }

    Application::~Application()
    {
        TE_CORE_INFO("Application Destructor called.");
    }

    void Application::Run()
    {
        TE_CORE_INFO("Application Run started.");

        WindowResizeEvent resizeEvent(1280, 720);
        TE_CLIENT_INFO("Event Triggered: " + resizeEvent.ToString());

        // Dummy run loop for testing
        bool running = true;
        int tick = 0;
        while (running)
        {
            TE_CORE_DEBUG("Tick #" + std::to_string(tick));
            ++tick;

            if (tick >= 5)  // simulate a break condition
            {
                running = false;
            }
        }

        TE_CORE_INFO("Application Run ended.");
    }
}
