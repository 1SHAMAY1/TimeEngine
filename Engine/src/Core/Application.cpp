#include "Application.h"

#include <glad/glad.h> // Only need GLAD, NOT <GL/gl.h>

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

        // Initialize GLAD after window + OpenGL context is created
        if (!gladLoadGLLoader((GLADloadproc)m_Window->GetGLLoaderFunction()))
        {
            TE_CORE_ERROR("Failed to initialize GLAD!");
            return;
        }

        TE_CORE_INFO("OpenGL Version: {0}", (const char*)glGetString(GL_VERSION));

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

        float time = 0.0f;

        while (m_Running)
        {
            time += 0.01f; // Adjust speed here

            float blue   = 0.5f + 0.5f * sin(time);     // oscillates between 0 and 1
            float red    = 0.2f + 0.2f * sin(time + 2); // slight red shift
            float green  = 0.2f + 0.2f * sin(time + 4); // slight green shift
            float alpha  = 1.0f; // Or use 0.5f + 0.5f * sin(time + offset) for transparent waves

            glClearColor(red, green, blue, alpha);
            glClear(GL_COLOR_BUFFER_BIT);

            if (m_Window)
                m_Window->OnUpdate();
        }

        TE_CORE_INFO("Application Run ended.");
    }
}
