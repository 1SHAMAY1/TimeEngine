#include "Core/PreRequisites.h"
#include "Application.h"
#include "Core/Asset/AssetManager.hpp"
#include "Core/Audio/AudioEngine.hpp"
#include "Core/Plugin/PluginManager.hpp"
#include "Core/Threading/Threading.hpp"
#include "Events/ApplicationEvent.h"
#include "Layers/ProfilingLayer.hpp"
#include "Layers/TimeGUILayer.hpp"
#include "Log.h"
#include "Renderer/RenderCommand.hpp"
#include "Renderer/TEColor.hpp"
#include "Utils/TimeGUI.hpp"
#include "Window/IWindow.hpp"

Application *Application::s_Instance = nullptr;

Application::Application() : m_Running(true)
{
    TE_CORE_ASSERT(!s_Instance, "Application already exists!");
    s_Instance = this;

    Log::Init(true, "TimeEngineLog.json");
    TE_CORE_INFO("Application Constructor called.");

    // Initialize Thread pools
    INIT_MAIN_THREAD();
    INIT_CALC_THREAD();
    INIT_AI_THREAD();
    INIT_WIDGET_THREAD();
    INIT_GAMEPLAY_THREAD();

    m_Window = IWindow::Create();

    if (!RenderCommand::LoadLoader((void *(*)(const char *))m_Window->GetGLLoaderFunction()))
    {
        TE_CORE_ERROR("Failed to initialize GLAD via RenderCommand!");
        m_Running = false;
        return;
    }

    TE_CORE_INFO("OpenGL Version: {0}", RenderCommand::GetVersionString());

#ifdef TE_EDITOR
    // === TimeGUI Layer Setup (Initialize while OpenGL context is active) ===
    m_TimeGUILayer = CreateRef<TimeGUILayer>();
    PushOverlay(m_TimeGUILayer);
#endif

    // Release context from Main Thread so Dedicated Render Thread exclusively owns it
    IWindow::MakeContextCurrent(nullptr);
    TaskSystem::InitRenderThread(m_Window->GetNativeWindow());

    m_Window->SetEventCallback(
        [this](Event &e)
        {
            for (auto it = m_LayerStack.end(); it != m_LayerStack.begin();)
            {
                if (e.Handled())
                    break;
                (*--it)->OnEvent(e);
            }

            if (e.Handled())
                return;

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
        });

    // Initialize Audio Subsystem
    AudioEngine::Initialize();

    // Initialize Plugins
    PluginManager::Initialize();
}

Application::~Application()
{
    TE_CORE_INFO("Application Destructor called.");
}

void Application::Close() { m_Running = false; }

void Application::Run()
{
    TE_CORE_INFO("Application Run started.");

    float time = 0.0f;

    while (m_Running)
    {
        auto *profiler = ProfilingLayer::GetInstance();
        if (profiler)
            profiler->OnUpdate();

        // 1. Application & Layer Logic update on Main Thread
        auto gameStartTime = std::chrono::high_resolution_clock::now();
        OnUpdate();
        for (Layer *layer : m_LayerStack)
        {
            if (layer)
                layer->OnUpdate();
        }
        auto gameEndTime = std::chrono::high_resolution_clock::now();
        if (profiler)
        {
            float gameDurationMs = std::chrono::duration<float, std::milli>(gameEndTime - gameStartTime).count();
            profiler->RecordGameTime(gameDurationMs);
        }

        // 2. Submit GPU / UI Frame to Dedicated Render Thread
        TaskSystem::SubmitRenderFrame([this]() {
            RenderCommand::SetClearColor(TEColor::Black());
            RenderCommand::Clear();

#ifdef TE_EDITOR
            m_TimeGUILayer->Begin();
            for (Layer *layer : m_LayerStack)
            {
                if (layer)
                    layer->OnTimeGUIRender();
            }
            m_TimeGUILayer->End();
#endif
        });

        // 3. Process deferred layer modifications
        m_LayerStack.ProcessDeferredRemovals();
        ProcessDeferredAdditions();

        // 4. Poll OS / Window events on Main Thread
        m_Window->OnUpdate();

        // 5. Sync with Render Thread frame
        TaskSystem::WaitRenderFrame();
    }

    TE_CORE_INFO("Application Run ended. Cleaning up subsystems...");

    // 1. Detach & release all active layers
    m_LayerStack.Clear();
    m_LayersToAdd.Clear();
    m_OverlaysToAdd.Clear();

    // 2. Shut down assets, plugins, and audio
    AssetManager::Shutdown();
    PluginManager::Shutdown();
    AudioEngine::Shutdown();

    // 3. Shut down dedicated render thread before destroying window
    TaskSystem::ShutdownRenderThread();

    // 4. Destroy window and terminate OpenGL/GLFW
    m_Window.reset();
    IWindow::Terminate();
}

void Application::PushLayer(TERef<Layer> layer) { m_LayerStack.PushLayer(layer); }

void Application::PushOverlay(TERef<Layer> overlay) { m_LayerStack.PushOverlay(overlay); }

void Application::PopLayer(TERef<Layer> layer) { m_LayerStack.PopLayer(layer); }

void Application::PopOverlay(TERef<Layer> overlay) { m_LayerStack.PopOverlay(overlay); }

void Application::MarkLayerForRemoval(TERef<Layer> layer) { m_LayerStack.MarkLayerForRemoval(layer); }

void Application::MarkOverlayForRemoval(TERef<Layer> overlay) { m_LayerStack.MarkOverlayForRemoval(overlay); }

void Application::MarkLayerForAddition(TERef<Layer> layer)
{
    if (layer)
        m_LayersToAdd.Add(layer);
}

void Application::MarkOverlayForAddition(TERef<Layer> overlay)
{
    if (overlay)
        m_OverlaysToAdd.Add(overlay);
}

void Application::ProcessDeferredAdditions()
{
    // Process layers to add
    for (const auto &layer : m_LayersToAdd)
    {
        if (layer)
            PushLayer(layer);
    }
    m_LayersToAdd.Empty();

    // Process overlays to add
    for (const auto &overlay : m_OverlaysToAdd)
    {
        if (overlay)
            PushOverlay(overlay);
    }
    m_OverlaysToAdd.Empty();
}

