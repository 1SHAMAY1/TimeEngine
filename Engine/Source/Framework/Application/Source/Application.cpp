#include "PreRequisites.h"
#include "Application.h"
#include "AssetManager.hpp"
#include "Audio/AudioEngine.hpp"
#include "Plugin/PluginManager.hpp"
#include "Settings/GeneralEngineSettings.hpp"
#include "Threading/Threading.hpp"
#include "Events/ApplicationEvent.h"
#include "ShortcutManager.hpp"
#include "Layers/ProfilingLayer.hpp"
#include "Layers/TimeGUILayer.hpp"
#include "Log.h"
#include "RenderCommand.hpp"
#include "RendererContext.hpp"
#include "TEColor.hpp"
#include "MathEngine.hpp"
#include "Utils/TEFileSystem.hpp"
#include "TimeGUI.hpp"
#include "IWindow.hpp"

Application *Application::s_Instance = nullptr;

Application::Application() : m_Running(true)
{
    TE_CORE_ASSERT(!s_Instance, "Application already exists!");
    s_Instance = this;

    Log::Init(true, "TimeEngineLog.json");
    MathEngine::Get().Initialize();
    TE_CORE_INFO("Application Constructor called.");

    // Load saved EngineSettings.ini before creating the window or initializing graphics contexts
    auto &settings = GeneralEngineSettings::Get();
    TEString configPath = settings.GetConfigPath() + "EngineSettings.ini";
    if (TEFileSystem::Exists(configPath))
    {
        settings.LoadFromFile(configPath);
    }
    else if (TEFileSystem::Exists("EngineSettings.ini"))
    {
        settings.LoadFromFile("EngineSettings.ini");
    }

    // Initialize Thread pools
    INIT_MAIN_THREAD();
    INIT_CALC_THREAD();
    INIT_AI_THREAD();
    INIT_WIDGET_THREAD();
    INIT_GAMEPLAY_THREAD();

    m_Window = IWindow::Create();

    switch (RendererContext::GetAPI())
    {
    case GraphicsAPI::DirectX11:
        TE_CORE_INFO("Graphics Backend: DirectX 11 (Version: {0}, GPU: {1})", RenderCommand::GetVersionString(),
                     RenderCommand::GetGPURenderer());
        break;
    case GraphicsAPI::OpenGL:
    case GraphicsAPI::OpenGLES:
#if defined(TE_SUPPORT_OPENGL)
        if (!RenderCommand::LoadLoader((void *(*)(const char *))m_Window->GetGLLoaderFunction()))
        {
            TE_CORE_ERROR("Failed to initialize GLAD via RenderCommand!");
            m_Running = false;
            return;
        }
        TE_CORE_INFO("Graphics Backend: OpenGL (Version: {0}, GPU: {1})", RenderCommand::GetVersionString(),
                     RenderCommand::GetGPURenderer());
#else
        TE_CORE_ERROR("OpenGL requested but not supported in this build!");
#endif
        break;
    case GraphicsAPI::Metal:
        TE_CORE_INFO("Graphics Backend: Metal (Version: {0}, GPU: {1})", RenderCommand::GetVersionString(),
                     RenderCommand::GetGPURenderer());
        break;
    case GraphicsAPI::Vulkan:
        TE_CORE_INFO("Graphics Backend: Vulkan (Version: {0}, GPU: {1})", RenderCommand::GetVersionString(),
                     RenderCommand::GetGPURenderer());
        break;
    case GraphicsAPI::None:
    default:
        TE_CORE_WARN("Graphics Backend: Unknown / None");
        break;
    }

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

Application::~Application()
{
    MathEngine::Get().Shutdown();
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
            [this, drawData, winW, winH, profiler]()
            {
                auto renderStartTime = std::chrono::high_resolution_clock::now();
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
                auto renderEndTime = std::chrono::high_resolution_clock::now();
                if (profiler)
                {
                    float renderDurationMs =
                        std::chrono::duration<float, std::milli>(renderEndTime - renderStartTime).count();
                    profiler->RecordRenderTime(renderDurationMs);
                }
            });
        TaskSystem::WaitRenderFrame(); // Wait for Render Thread before next frame starts

        // Show window once the first frame has been rendered and presented
        if (m_Window && !m_Window->IsVisible())
            m_Window->ShowWindow();

        // 4. Stage 3: Deferred layer modifications
        m_LayerStack.ProcessDeferredRemovals();
        ProcessDeferredAdditions();
    }

    TE_CORE_INFO("Application Run ended. Cleaning up subsystems...");

    // 1. Shut down dedicated worker threads FIRST (flushes pending render/widget jobs before subsystems are destroyed)
    TaskSystem::ShutdownWidgetThread();
    TaskSystem::ShutdownRenderThread();

    // 2. Release asset prototypes while plugin DLL code is still loaded in memory
    AssetManager::Shutdown();

    // 3. Shut down plugins and audio while layers are intact
    PluginManager::Shutdown();
    AudioEngine::Shutdown();

    // 4. Detach & release all active layers
    m_LayerStack.Clear();
    m_LayersToAdd.Clear();
    m_OverlaysToAdd.Clear();

    // 5. Shut down shortcuts after threads and layers are done
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
