#include "Application.h"
#include "Core/Threading/ThreadingMacros.hpp"
#include "Events/ApplicationEvent.h"
#include "Layers/TimeGUILayer.hpp"
#include "Log.h"
#include "Renderer/RenderCommand.hpp"
#include "Renderer/TEColor.hpp"
#include "Utils/TimeGUI.hpp"
#include "Window/IWindow.hpp"
#include "Core/Plugin/PluginManager.hpp"

namespace TE
{
Application *Application::s_Instance = nullptr;

Application::Application() : m_Running(true)
{
    TE_CORE_ASSERT(!s_Instance, "Application already exists!");
    s_Instance = this;

    TE::Log::Init(true, "TimeEngineLog.json");
    TE_CORE_INFO("Application Constructor called.");

    // Initialize Thread pools
    INIT_MAIN_THREAD();
    INIT_RENDER_THREAD();
    INIT_CALC_THREAD();
    INIT_AI_THREAD();
    INIT_WIDGET_THREAD();
    INIT_GAMEPLAY_THREAD();

    m_Window = std::unique_ptr<IWindow>(IWindow::Create());

    if (!RenderCommand::LoadLoader((void *(*)(const char *))m_Window->GetGLLoaderFunction()))
    {
        TE_CORE_ERROR("Failed to initialize GLAD via RenderCommand!");
        return;
    }

    TE_CORE_INFO("OpenGL Version: {0}", RenderCommand::GetVersionString());

    m_Window->SetEventCallback(
        [this](Event &e)
        {
            EventDispatcher dispatcher(e);

            dispatcher.Dispatch<WindowCloseEvent>(
                [this](WindowCloseEvent &event)
                {
                    TE_CLIENT_INFO("Window close event received.");
                    m_Running = false;
                    return true;
                });

            dispatcher.Dispatch<WindowResizeEvent>(
                [](WindowResizeEvent &event)
                {
                    TE_CLIENT_INFO("Window resized: " + event.ToString());
                    return false;
                });

            dispatcher.Dispatch<WindowFocusEvent>(
                [](WindowFocusEvent &event)
                {
                    TE_CLIENT_INFO("Window gained focus");
                    return false;
                });

            dispatcher.Dispatch<WindowLostFocusEvent>(
                [](WindowLostFocusEvent &event)
                {
                    TE_CLIENT_INFO("Window lost focus");
                    return false;
                });

            for (auto it = m_LayerStack.end(); it != m_LayerStack.begin();)
            {
                if (e.Handled())
                    break;
                (*--it)->OnEvent(e);
            }
        });

#ifdef TE_EDITOR
    // === TimeGUI Layer Setup ===
    m_TimeGUILayer = new TimeGUILayer();
    PushOverlay(m_TimeGUILayer);
#endif

    // Initialize Plugins
    PluginManager::Initialize();
}

Application::~Application()
{
    PluginManager::Shutdown();
    TE_CORE_INFO("Application Destructor called.");
}

void Application::Close() { m_Running = false; }

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
        for (Layer *layer : m_LayerStack)
        {
            if (layer)
                layer->OnUpdate();
        }

#ifdef TE_EDITOR
        // TimeGUI Rendering
        m_TimeGUILayer->Begin();
        for (Layer *layer : m_LayerStack)
        {
            if (layer)
                layer->OnTimeGUIRender();
        }
        m_TimeGUILayer->End();
#endif

        // Process any deferred layer removals after all layer operations are complete
        m_LayerStack.ProcessDeferredRemovals();

        // Process any deferred layer additions
        ProcessDeferredAdditions();

        m_Window->OnUpdate();
    }

    TE_CORE_INFO("Application Run ended.");

    IWindow::Terminate();
}

void Application::PushLayer(Layer *layer) { m_LayerStack.PushLayer(layer); }

void Application::PushOverlay(Layer *overlay) { m_LayerStack.PushOverlay(overlay); }

void Application::PopLayer(Layer *layer) { m_LayerStack.PopLayer(layer); }

void Application::PopOverlay(Layer *overlay) { m_LayerStack.PopOverlay(overlay); }

void Application::MarkLayerForRemoval(Layer *layer) { m_LayerStack.MarkLayerForRemoval(layer); }

void Application::MarkOverlayForRemoval(Layer *overlay) { m_LayerStack.MarkOverlayForRemoval(overlay); }

void Application::MarkLayerForAddition(Layer *layer)
{
    if (layer)
        m_LayersToAdd.push_back(layer);
}

void Application::MarkOverlayForAddition(Layer *overlay)
{
    if (overlay)
        m_OverlaysToAdd.push_back(overlay);
}

void Application::ProcessDeferredAdditions()
{
    // Process layers to add
    for (Layer *layer : m_LayersToAdd)
    {
        if (layer)
            PushLayer(layer);
    }
    m_LayersToAdd.clear();

    // Process overlays to add
    for (Layer *overlay : m_OverlaysToAdd)
    {
        if (overlay)
            PushOverlay(overlay);
    }
    m_OverlaysToAdd.clear();
}
} // namespace TE
