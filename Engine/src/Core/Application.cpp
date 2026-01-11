#include "Application.h"

#include <glad/glad.h>

#include "Events/ApplicationEvent.h"
#include "Log.h"
#include "Window/IWindow.hpp"
#include "ImGUI/ImGuiLayer.hpp"
#include "Renderer/TEColor.hpp"
#include "Renderer/RenderCommand.hpp"


namespace TE
{
    Application* Application::s_Instance = nullptr;

    Application::Application()
        : m_Running(true)
    {
        TE_CORE_ASSERT(!s_Instance, "Application already exists!");
        s_Instance = this;

        TE::Log::Init(true, "TimeEngineLog.json");
        TE_CORE_INFO("Application Constructor called.");

        m_Window = std::unique_ptr<IWindow>(IWindow::Create());

        if (!gladLoadGLLoader((GLADloadproc)m_Window->GetGLLoaderFunction()))
        {
            TE_CORE_ERROR("Failed to initialize GLAD!");
            return;
        }

        TE_CORE_INFO("OpenGL Version: {0}", (const char*)glGetString(GL_VERSION));

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

        

        // === ImGui Layer Setup ===
        m_ImGuiLayer = new ImGuiLayer();
        PushOverlay(m_ImGuiLayer);
        
        
    }

    Application::~Application()
    {
        TE_CORE_INFO("Application Destructor called.");
    }

    void Application::Close()
    {
        m_Running = false;
    }

    void Application::Run()
    {
        TE_CORE_INFO("Application Run started.");

        float time = 0.0f;

        while (m_Running)
        {
            RenderCommand::SetClearColor(TEColor::Black());
            RenderCommand::Clear();
            
            // Application update
            OnUpdate();
            
            // Logic update
            for (Layer* layer : m_LayerStack)
            {
                if (layer)
                    layer->OnUpdate();
            }

            // ImGui Rendering
            m_ImGuiLayer->Begin();
            for (Layer* layer : m_LayerStack)
            {
                if (layer)
                    layer->OnImGuiRender();
            }
            m_ImGuiLayer->End();

            // Process any deferred layer removals after all layer operations are complete
            m_LayerStack.ProcessDeferredRemovals();
            
            // Process any deferred layer additions
            ProcessDeferredAdditions();

            m_Window->OnUpdate();
        }

        TE_CORE_INFO("Application Run ended.");
    }

    void Application::PushLayer(Layer* layer)
    {
        m_LayerStack.PushLayer(layer);
    }

    void Application::PushOverlay(Layer* overlay)
    {
        m_LayerStack.PushOverlay(overlay);
    }

    void Application::PopLayer(Layer* layer)
    {
        m_LayerStack.PopLayer(layer);
    }

    void Application::PopOverlay(Layer* overlay)
    {
        m_LayerStack.PopOverlay(overlay);
    }

    void Application::MarkLayerForRemoval(Layer* layer)
    {
        m_LayerStack.MarkLayerForRemoval(layer);
    }


    void Application::MarkOverlayForRemoval(Layer* overlay)
    {
        m_LayerStack.MarkOverlayForRemoval(overlay);
    }

    void Application::MarkLayerForAddition(Layer* layer)
    {
        if (layer)
            m_LayersToAdd.push_back(layer);
    }

    void Application::MarkOverlayForAddition(Layer* overlay)
    {
        if (overlay)
            m_OverlaysToAdd.push_back(overlay);
    }

    void Application::ProcessDeferredAdditions()
    {
        // Process layers to add
        for (Layer* layer : m_LayersToAdd)
        {
            if (layer)
                PushLayer(layer);
        }
        m_LayersToAdd.clear();
        
        // Process overlays to add
        for (Layer* overlay : m_OverlaysToAdd)
        {
            if (overlay)
                PushOverlay(overlay);
        }
        m_OverlaysToAdd.clear();
    }
}
