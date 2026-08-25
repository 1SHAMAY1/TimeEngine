#pragma once

inline size_t GetThreadCountForPercentage(float percentage) {
    unsigned int total = std::thread::hardware_concurrency();
    if (total == 0) total = 4; // fallback
    size_t count = static_cast<size_t>((total * percentage) / 100.0f);
    return count > 0 ? count : 1;
}

#include "Window/IWindow.hpp"

class DedicatedRenderThread
{
public:
    DedicatedRenderThread(void *nativeWindow)
        : m_NativeWindow(nativeWindow), m_Running(true)
    {
        m_Thread = std::thread(&DedicatedRenderThread::ThreadLoop, this);
    }

    ~DedicatedRenderThread()
    {
        Shutdown();
    }

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
        m_DoneCV.wait(lock, [this]() { return m_FrameDone || !m_Running; });
    }

private:
    void ThreadLoop()
    {
        if (m_NativeWindow)
        {
            IWindow::MakeContextCurrent(m_NativeWindow);
        }

        while (m_Running)
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

inline void TaskSystem::InitMainThread() {
    threadEnabled[TaskType::MAIN] = true;
}

inline void TaskSystem::InitRenderThread(void *nativeWindow) {
    threadEnabled[TaskType::RENDER] = true;
    if (nativeWindow)
    {
        s_RenderThread = CreateScope<DedicatedRenderThread>(nativeWindow);
    }
}

inline void TaskSystem::ShutdownRenderThread() {
    if (s_RenderThread)
    {
        s_RenderThread->Shutdown();
        s_RenderThread.reset();
    }
}

inline void TaskSystem::SubmitRenderFrame(const std::function<void()>& renderJob) {
    if (s_RenderThread)
        s_RenderThread->SubmitFrame(renderJob);
    else
        renderJob();
}

inline void TaskSystem::WaitRenderFrame() {
    if (s_RenderThread)
        s_RenderThread->WaitFrame();
}

inline void TaskSystem::InitGameplayThread() {
    threadEnabled[TaskType::GAMEPLAY] = true;
    threadPools[TaskType::GAMEPLAY] = CreateScope<ThreadPool>(GetThreadCountForPercentage(3.0f));
}

inline void TaskSystem::InitAIThread() {
    threadEnabled[TaskType::AI] = true;
    threadPools[TaskType::AI] = CreateScope<ThreadPool>(GetThreadCountForPercentage(10.0f));
}

inline void TaskSystem::InitCalcThread() {
    threadEnabled[TaskType::CALC] = true;
    threadPools[TaskType::CALC] = CreateScope<ThreadPool>(GetThreadCountForPercentage(2.0f));
}

inline void TaskSystem::InitWidgetThread() {
    threadEnabled[TaskType::WIDGET] = true;
    threadPools[TaskType::WIDGET] = CreateScope<ThreadPool>(GetThreadCountForPercentage(5.0f));
}

inline void TaskSystem::SetThreadEnabled(TaskType type, bool enabled) {
    threadEnabled[type] = enabled;
}

inline void TaskSystem::RestartThread(TaskType type) {
    if (type == TaskType::MAIN) return;

    if (threadPools.Find(type) != nullptr)
        threadPools[type].reset();

    switch (type) {
        case TaskType::RENDER:
            break;
        case TaskType::GAMEPLAY:
            threadPools[type] = CreateScope<ThreadPool>(GetThreadCountForPercentage(3.0f));
            break;
        case TaskType::AI:
            threadPools[type] = CreateScope<ThreadPool>(GetThreadCountForPercentage(10.0f));
            break;
        case TaskType::CALC:
            threadPools[type] = CreateScope<ThreadPool>(GetThreadCountForPercentage(2.0f));
            break;
        case TaskType::WIDGET:
            threadPools[type] = CreateScope<ThreadPool>(GetThreadCountForPercentage(5.0f));
            break;
        default:
            break;
    }

    threadEnabled[type] = true;
}

inline void TaskSystem::Submit(TaskType type, const std::function<void()>& job) {
    auto *enabled = threadEnabled.Find(type);
    if (!enabled || !*enabled) return;

    if (type == TaskType::MAIN) {
        job();
        return;
    }

    if (type == TaskType::RENDER) {
        SubmitRenderFrame(job);
        return;
    }

    auto *pool = threadPools.Find(type);
    if (pool && *pool)
        (*pool)->Enqueue(job);
}
