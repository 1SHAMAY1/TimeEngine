#include "Application.h"
#include "Events/ApplicationEvent.h"
#include "Log.h"
#include "Window/IWindow.hpp"

namespace TE
{
    Application::Application()
        : m_Running(true)
    {
        // Initialize the logging system once
        TE::Log::Init(true, "TimeEngineLog.json");

        TE_CORE_INFO("Application Constructor called.");

        // Create a window instance via abstraction
        m_Window = std::unique_ptr<IWindow>(IWindow::Create());

        // Set up event callback and dispatch relevant events
        m_Window->SetEventCallback([this](Event& e) {
            EventDispatcher dispatcher(e);

            dispatcher.Dispatch<WindowCloseEvent>([this](WindowCloseEvent& event) {
                TE_CLIENT_INFO("Window close event received.");
                m_Running = false;
                return true;
            });

            dispatcher.Dispatch<WindowResizeEvent>([](WindowResizeEvent& event) {
                TE_CLIENT_INFO("Window resized: " + event.ToString());
                return false;
            });

            dispatcher.Dispatch<WindowFocusEvent>([](WindowFocusEvent& event) {
                TE_CLIENT_INFO("Window gained focus");
                return false;
            });

            dispatcher.Dispatch<WindowLostFocusEvent>([](WindowLostFocusEvent& event) {
                TE_CLIENT_INFO("Window lost focus");
                return false;
            });
        });
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

        int tick = 0;
        while (m_Running)
        {
            if (m_Window)
                m_Window->OnUpdate();

            // TE_CORE_DEBUG("Tick #" + std::to_string(tick));
            ++tick;
        }

        TE_CORE_INFO("Application Run ended.");
    }
} // namespace TE
