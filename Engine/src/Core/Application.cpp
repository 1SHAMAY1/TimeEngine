#include "Application.h"
#include "Core/Asset/AssetManager.hpp"
#include "Core/Audio/AudioEngine.hpp"
#include "Core/Plugin/PluginManager.hpp"
#include "Core/PreRequisites.h"
#include "Core/Threading/Threading.hpp"
#include "Events/ApplicationEvent.h"
#include "Input/ShortcutManager.hpp"
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
    TaskSystem::InitWidgetThread();
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

Application::~Application() { TE_CORE_INFO("Application Destructor called."); }

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

        // 1. Poll OS / Window events & Game Logic on Main Thread
        m_Window->OnUpdate();

#ifdef TE_EDITOR
        TimeGUI::PrepareGLFWFrame();
#endif

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

        void *drawData = nullptr;

#ifdef TE_EDITOR
        // 2. Stage 1: UI Generation on Dedicated Widget Thread
        TaskSystem::SubmitWidgetFrame(
            [this, &drawData, profiler]()
            {
                auto uiStartTime = std::chrono::high_resolution_clock::now();
                m_TimeGUILayer->Begin();
                for (Layer *layer : m_LayerStack)
                {
                    if (layer)
                        layer->OnTimeGUIRender();
                }
                drawData = m_TimeGUILayer->End();
                auto uiEndTime = std::chrono::high_resolution_clock::now();
                if (profiler)
                {
                    float uiDurationMs = std::chrono::duration<float, std::milli>(uiEndTime - uiStartTime).count();
                    profiler->RecordUITime(uiDurationMs);
                }
            });
        TaskSystem::WaitWidgetFrame(); // Wait for Widget Thread to finish generating draw data
#endif

        // 3. Stage 2: GPU Rasterization on Dedicated Render Thread
        uint32_t winW = m_Window ? m_Window->GetWidth() : 1280;
        uint32_t winH = m_Window ? m_Window->GetHeight() : 720;

        TaskSystem::SubmitRenderFrame(
            [this, drawData, winW, winH]()
            {
                RenderCommand::SetViewport(0, 0, winW, winH);
                RenderCommand::SetClearColor(TEColor::Black());
                RenderCommand::Clear();

                // Execute layer scene render passes (framebuffers, Renderer2D, lights, etc.)
                for (Layer *layer : m_LayerStack)
                {
                    if (layer)
                        layer->OnRender();
                }

#ifdef TE_EDITOR
                if (drawData)
                    TimeGUI::RenderDrawData(drawData);
#endif
            });
        TaskSystem::WaitRenderFrame(); // Wait for Render Thread before next frame starts

        // 4. Stage 3: Deferred layer modifications
        m_LayerStack.ProcessDeferredRemovals();
        ProcessDeferredAdditions();
    }

    TE_CORE_INFO("Application Run ended. Cleaning up subsystems...");

    // 1. Release asset prototypes FIRST while plugin DLL code is still loaded in memory
    AssetManager::Shutdown();

    // 2. Shut down plugins and audio while layers are intact
    PluginManager::Shutdown();
    AudioEngine::Shutdown();

    // 3. Detach & release all active layers
    m_LayerStack.Clear();
    m_LayersToAdd.Clear();
    m_OverlaysToAdd.Clear();

    // 4. Shut down dedicated worker threads (flushes pending render/widget jobs)
    TaskSystem::ShutdownWidgetThread();
    TaskSystem::ShutdownRenderThread();

    // 5. Shut down shortcuts after threads are done
    ShortcutManager::Shutdown();

    // 6. Destroy window and terminate OpenGL/GLFW
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
