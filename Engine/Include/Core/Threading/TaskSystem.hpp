#pragma once
#include "Core/PreRequisites.h"
#include "GameFrameWork/GameplayUtils.hpp"
#include "ThreadPool.hpp"
#include "Window/IWindow.hpp"
#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>
#include <unordered_map>

#ifdef TE_EDITOR
#include "Utils/TimeGUI.hpp"
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

private:
    void ThreadLoop()
    {
        if (m_NativeWindow)
        {
            IWindow::MakeContextCurrent(m_NativeWindow);
#if defined(TE_EDITOR) && defined(TE_SUPPORT_OPENGL)
            TimeGUI::InitOpenGLBackend();
#endif
        }

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

            if (m_NativeWindow)
            {
                IWindow::SwapBuffers(m_NativeWindow);
            }

            {
                std::lock_guard<std::mutex> lock(m_Mutex);
                m_FrameDone = true;
            }
            m_DoneCV.notify_one();
        }

#if defined(TE_EDITOR) && defined(TE_SUPPORT_OPENGL)
        TimeGUI::ShutdownOpenGLBackend();
#endif
        IWindow::MakeContextCurrent(nullptr);
    }

    void *m_NativeWindow = nullptr;
    std::thread m_Thread;
    std::function<void()> m_Job = nullptr;
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
