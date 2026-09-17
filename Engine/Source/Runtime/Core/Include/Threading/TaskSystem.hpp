#pragma once
#include "PreRequisites.h"
#include "GameplayUtils.hpp"
#include "Threading/ThreadPool.hpp"
#include "IWindow.hpp"
#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>
#include <unordered_map>

#include "RenderCommand.hpp"
#include "RendererContext.hpp"

#ifdef TE_EDITOR
#include "TimeGUI.hpp"
#endif

inline size_t GetThreadCountForPercentage(float percentage)
{
    unsigned int total = std::thread::hardware_concurrency();
    if (total == 0)
        total = 4; // fallback
    size_t count = static_cast<size_t>((total * percentage) / 100.0f);
    return count > 0 ? count : 1;
}

class RenderThread
{
public:
    RenderThread(void *nativeWindow) : m_NativeWindow(nativeWindow), m_Running(true)
    {
        m_Thread = std::thread(&RenderThread::ThreadLoop, this);
    }

    ~RenderThread() { Shutdown(); }

    void Shutdown()
    {
        if (m_Running)
        {
            {
                std::lock_guard<std::mutex> lock(m_Mutex);
                m_Running = false;
            }
            m_WorkCV.notify_all();
            if (m_Thread.joinable())
                m_Thread.join();
        }
    }

    void SubmitFrame(const std::function<void()> &job)
    {
        {
            std::unique_lock<std::mutex> lock(m_Mutex);
            m_Job = job;
            m_HasWork = true;
            m_FrameDone = false;
        }
        m_WorkCV.notify_one();
    }

    void WaitFrame()
    {
        std::unique_lock<std::mutex> lock(m_Mutex);
        m_DoneCV.wait(lock, [this]() { return m_FrameDone; });
    }

    void ExecuteBlocking(const std::function<void()> &task)
    {
        if (!task)
            return;

        if (!m_Running || std::this_thread::get_id() == m_Thread.get_id())
        {
            task();
            return;
        }

        bool completed = false;
        std::mutex taskMutex;
        std::condition_variable taskCV;

        {
            std::unique_lock<std::mutex> lock(m_Mutex);
            m_PendingTasks.Add([&task, &completed, &taskMutex, &taskCV]() {
                task();
                {
                    std::lock_guard<std::mutex> lk(taskMutex);
                    completed = true;
                }
                taskCV.notify_one();
            });
        }
        m_WorkCV.notify_one();

        std::unique_lock<std::mutex> lk(taskMutex);
        taskCV.wait(lk, [&completed]() { return completed; });
    }

private:
    void ThreadLoop()
    {
        GraphicsAPI currentAPI = RendererContext::GetAPI();

        if (currentAPI == GraphicsAPI::OpenGL || currentAPI == GraphicsAPI::OpenGLES)
        {
#if defined(TE_SUPPORT_OPENGL)
            if (m_NativeWindow)
                IWindow::MakeContextCurrent(m_NativeWindow);
#endif
        }

#if defined(TE_EDITOR)
        TimeGUI::InitOpenGLBackend(); // routes to active render backend
#endif

        while (true)
        {
            std::function<void()> currentJob = nullptr;
            TEArray<std::function<void()>> tasks;
            {
                std::unique_lock<std::mutex> lock(m_Mutex);
                m_WorkCV.wait(lock, [this]() { return m_HasWork || !m_PendingTasks.IsEmpty() || !m_Running; });

                if (!m_Running)
                    break;

                tasks = std::move(m_PendingTasks);
                m_PendingTasks.Clear();

                if (m_HasWork)
                {
                    currentJob = std::move(m_Job);
                    m_Job = nullptr;
                    m_HasWork = false;
                }
            }

            for (size_t i = 0; i < tasks.Num(); ++i)
            {
                if (tasks[i])
                    tasks[i]();
            }

            if (currentJob)
            {
                currentJob();

                switch (currentAPI)
                {
                case GraphicsAPI::DirectX11:
#if defined(TE_SUPPORT_DIRECTX11) && defined(TE_PLATFORM_WINDOWS)
                    RenderCommand::Present();
#endif
                    break;
                case GraphicsAPI::OpenGL:
                case GraphicsAPI::OpenGLES:
#if defined(TE_SUPPORT_OPENGL)
                    if (m_NativeWindow)
                    {
                        IWindow::SwapBuffers(m_NativeWindow);
                    }
#endif
                    break;
                default:
                    break;
                }

                {
                    std::lock_guard<std::mutex> lock(m_Mutex);
                    m_FrameDone = true;
                }
                m_DoneCV.notify_one();
            }
        }

#if defined(TE_EDITOR)
        TimeGUI::ShutdownOpenGLBackend();
#endif
        if (currentAPI == GraphicsAPI::OpenGL || currentAPI == GraphicsAPI::OpenGLES)
        {
#if defined(TE_SUPPORT_OPENGL)
            IWindow::MakeContextCurrent(nullptr);
#endif
        }
    }

    void *m_NativeWindow = nullptr;
    std::thread m_Thread;
    std::function<void()> m_Job = nullptr;
    TEArray<std::function<void()>> m_PendingTasks;
    std::mutex m_Mutex;
    std::condition_variable m_WorkCV;
    std::condition_variable m_DoneCV;
    bool m_Running = false;
    bool m_HasWork = false;
    bool m_FrameDone = true;
};

class WidgetThread
{
public:
    WidgetThread() : m_Running(true) { m_Thread = std::thread(&WidgetThread::ThreadLoop, this); }

    ~WidgetThread() { Shutdown(); }

    void Shutdown()
    {
        if (m_Running)
        {
            {
                std::lock_guard<std::mutex> lock(m_Mutex);
                m_Running = false;
            }
            m_WorkCV.notify_all();
            if (m_Thread.joinable())
                m_Thread.join();
        }
    }

    void SubmitFrame(const std::function<void()> &job)
    {
        {
            std::unique_lock<std::mutex> lock(m_Mutex);
            m_Job = job;
            m_HasWork = true;
            m_FrameDone = false;
        }
        m_WorkCV.notify_one();
    }

    void WaitFrame()
    {
        std::unique_lock<std::mutex> lock(m_Mutex);
        m_DoneCV.wait(lock, [this]() { return m_FrameDone; });
    }

private:
    void ThreadLoop()
    {
        while (true)
        {
            std::function<void()> currentJob = nullptr;
            {
                std::unique_lock<std::mutex> lock(m_Mutex);
                m_WorkCV.wait(lock, [this]() { return m_HasWork || !m_Running; });

                if (!m_Running)
                    break;

                currentJob = std::move(m_Job);
                m_Job = nullptr;
                m_HasWork = false;
            }

            if (currentJob)
            {
                currentJob();
            }

            {
                std::lock_guard<std::mutex> lock(m_Mutex);
                m_FrameDone = true;
            }
            m_DoneCV.notify_one();
        }
    }

    std::thread m_Thread;
    std::function<void()> m_Job = nullptr;
    std::mutex m_Mutex;
    std::condition_variable m_WorkCV;
    std::condition_variable m_DoneCV;
    bool m_Running = false;
    bool m_HasWork = false;
    bool m_FrameDone = true;
};

enum class TaskType
{
    MAIN,
    RENDER,
    GAMEPLAY,
    AI,
    CALC,
    WIDGET
};

class TaskSystem
{
public:
    static void Submit(TaskType type, const std::function<void()> &job);
    static void SetThreadEnabled(TaskType type, bool enabled);
    static void RestartThread(TaskType type);

    // Initialization
    static void InitMainThread();
    static void InitRenderThread(void *nativeWindow = nullptr);
    static void ShutdownRenderThread();
    static void SubmitRenderFrame(const std::function<void()> &renderJob);
    static void WaitRenderFrame();
    static void ExecuteOnRenderThreadBlocking(const std::function<void()> &task);
    static void InitGameplayThread();
    static void InitAIThread();
    static void InitCalcThread();
    static void InitWidgetThread();
    static void ShutdownWidgetThread();
    static void SubmitWidgetFrame(const std::function<void()> &widgetJob);
    static void WaitWidgetFrame();

private:
    inline static TEMap<TaskType, TEScope<ThreadPool>> threadPools;
    inline static TEMap<TaskType, bool> threadEnabled;
    inline static TEScope<RenderThread> s_RenderThread;
    inline static TEScope<WidgetThread> s_WidgetThread;
};

#include "TaskSystem.inl"
